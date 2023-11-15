/*
   Night Time Podcast Player By John Melki-Wegner
   Has an issue with using
   DFPlayer.fastForward() and
   DFPlayer.setPlayTime()
   Both are unreliable when values of seconds are bigger than one byte!!!
   Supposed to be okay for unsigned int.
   Compromise:
   DFPlayer.fastForward() through file in increments of 200 to place the playhead where I want it

*/

#include <EEPROM.h>
#include <DFRobot_DF1201S.h>
#include <SoftwareSerial.h>
#include <Button.h>

// ======= Hardware ===============
Button button7(7); // Connect button between pin 7 and GND - This is the play button
Button button12(12); // Connect button between pin 7 and GND - This is the Reset button

SoftwareSerial DF1201SSerial(2, 3);  //RX  TX

DFRobot_DF1201S DFPlayer;

// ======= Pot ====================
int potValue = 0;
int volume;

// ======== Timer =================
uint16_t tenMinutes = 600; //seconds in 10 minutes
long t1 = 0, t2 = 0, t3 = 0;
bool sleepTimerOn = false;

// ======== File Info =============
uint16_t currentFileNumber = 0;
uint16_t currentTimeIndex = 0; //counts blocks of ten minutes
uint16_t numberOfFiles = 0;

// ========== Set Up ==============

void setup(void) {
  //Serial Monitor
  Serial.begin(9600);

  Serial.println("====================================");

  //DFPlayer Serial
  DF1201SSerial.begin(9600);
  while (!DFPlayer.begin(DF1201SSerial)) {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }

  //Switch Input
  button7.begin();
  button12.begin();

  // ---- Setup Player ------------------------
  /*Set volume*/
  DFPlayer.setVol(1);
  Serial.print("VOL:");
  /*Get volume*/
  Serial.println(DFPlayer.getVol());
  /*Enter music mode*/
  DFPlayer.switchFunction(DFPlayer.MUSIC);
  /*Set playback mode to "repeat all"*/
  DFPlayer.setPlayMode(DFPlayer.ALLCYCLE);

  Serial.print("numberOfFiles: ");
  numberOfFiles = DFPlayer.getTotalFile();
  Serial.println(numberOfFiles);

  //Get currentFileNumber and currentTimeIndex from EEProm
  //Even though EEPROM reads and writes a byte to and uint16_t 
  //it works okay as these are always below 255
  currentFileNumber = EEPROM.read(0);
  currentTimeIndex = EEPROM.read(1);
  //Check if file index is too big.
  if (currentFileNumber > numberOfFiles - 1) currentFileNumber = 0;
  Serial.print("File Number: ");
  Serial.println(currentFileNumber);
  Serial.print("Time Index: ");
  Serial.println(currentTimeIndex);
  Serial.println("====================================");
}

void loop() {
  readPot();
  checkButtons();
  sleepTimer();
}

void readPot() {
  potValue = analogRead(A0);
  volume = map(potValue, 0, 1023, 0, 20); //15 is plenty loud enough for me
  DFPlayer.setVol(volume);
}

void checkButtons() {
  if (button7.pressed()) {
    if (sleepTimerOn) pausePlay();
    else playPodcast();//Start play and Timer
  }
  if (button12.pressed()) gotoNextFile();
}

void pausePlay() {
  Serial.println("Pause");
  sleepTimerOn = false;
  DFPlayer.pause();
}

void gotoNextFile() {
  Serial.println("Next 10 minutes");
  currentTimeIndex++;
  EEPROM.update(1, currentTimeIndex);
  playPodcast();
}

void playPodcast() {
  DFPlayer.enableAMP();
  t1 = millis();
  sleepTimerOn = true;
  //run out of files??
  if (currentFileNumber >= numberOfFiles - 1) currentFileNumber = 0;
  //Make sure saved file number is played
  while (DFPlayer.getCurFileNumber() < currentFileNumber) {
    DFPlayer.next();
    currentTimeIndex = 0;
    EEPROM.update(1, currentTimeIndex);
  }
  Serial.print("File Number: ");
  Serial.print(DFPlayer.getCurFileNumber());
  Serial.print("\tcurrentTimeIndex ");
  Serial.println(currentTimeIndex);

  Serial.print("Start at: ");
  uint16_t startAt = tenMinutes * currentTimeIndex;
  Serial.println(startAt);
  DFPlayer.setPlayTime(0);
  DFPlayer.setVol(0); //Set volume so skiping FF noise not heard
  setTimeUsingFF(startAt);
  DFPlayer.start();
}

void setTimeUsingFF(int startAt) {
  //Doing this because of a bug in software that doesn't allow numbers bigger than 255
  //Past End of file?
  Serial.print("File size: ");
  Serial.println(DFPlayer.getTotalTime());
  if (startAt > DFPlayer.getTotalTime()) {
    Serial.println("Past end of file.");
    currentFileNumber++;
    DFPlayer.next();
    currentTimeIndex = 0;
    return;
  }
  //Potentially before start of file?
  if (startAt < 200) return;
  //Move play head to correct place - Using shifts of 200 as it is less than 255
  while (DFPlayer.getCurTime() < startAt - 200) {
    DFPlayer.fastForward(200);
  }
}

void sleepTimer() {
  t2 = millis();
  //show count down
  if (sleepTimerOn) {
    int tmp = (long(tenMinutes) - (t2 - t1) / 1000);
    if (t2 - t3 > 10000) {
      Serial.print(DFPlayer.getCurTime());
      Serial.print("\t");
      Serial.println(tmp);
      t3 = t2;
    }
  }

  //Compare seconds
  if (sleepTimerOn && (t2 - t1) / 1000 > long(tenMinutes)) {
    sleepTimerOn = false;
    Serial.println("Timer Off");
    DFPlayer.pause();
    DFPlayer.disableAMP(); //Save power?
    currentTimeIndex++; //Ten minutes has passed so increment for next restart
    //Reached end of current file?
    //go to next file
    if (tenMinutes * uint16_t(currentTimeIndex) > DFPlayer.getTotalTime()) {
      currentFileNumber++;
      currentTimeIndex = 0;
    }
    //Save to EEPROM
    EEPROM.update(0, currentFileNumber);
    EEPROM.update(1, currentTimeIndex);
  }
}
