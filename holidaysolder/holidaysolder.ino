#include <EEPROM.h>
#include <avr/sleep.h>

int ledBlu = 0;
int ledGrn = 1;
int ledRed = 2;
int speakerPin = A2;

int switcher = 0;

//number of notes in song
int length = 117;
//notes
char notes[] = "gefgCbCDCbagbCDCbaagCegagfefggefgCbCDCbagbCDCbaagCegagfedccaaCCbagefagfeeddggbbDbCDCbaggefgCbCDCbagbCDCbaagCegaggabC ";
//length of each note
int beats[] = { 4,3,1,2,4,1,1,2,2,2,2,6,1,1,2,2,2,1,1,2,2,2,1,1,2,2,2,2,8,4,3,1,2,4,1,1,2,2,2,2,6,1,1,2,2,2,1,1,2,2,2,1,1,2,2,2,2,6, 2,2,2,2,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,2,2,1,1,2,2,2,2,6 ,4,3,1,2,4,1,1,2,2,2,2,6,1,1,2,2,2,1,1,2,2,2,1,1,4,4,4,4,8,8};
//tempo
int tempo = 140;

//shuts down unneeded functions to save battery when not in use - acts as a mode
void sleepNow()
{
  DDRB  = B000000;
  PORTB = B100000;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();
}

//reference pointer used to keep track of what mode we are in - stored in eeprom
typedef void(*ModePtr)();
ModePtr ModePtrs[] =
{
  sleepNow,
  playFrosty
};

//number of modes, initialize mode
const static byte mode_count = sizeof(ModePtrs) / sizeof(ModePtrs[0]);
static byte mode = 0;

//function to play a tone at a set frequency for a given duration
void playTone(int tone, int duration)
{
  for (long i = 0; i < duration * 1000L; i += tone * 2)
  {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

//function to play a note for a duration - calls playTone
void playNote(char note, int duration)
{
  //these two arrays allow us to get the frequency associated with a letter (note)
  char names[] = { 'c', 'd', 'e', 'f', 's', 'g', 'a', 'v', 'b', 'C', 'D', 'E' , 'F'};
  int tones[] = { 1915, 1700, 1519, 1432, 1352, 1275, 1136, 1073, 1014, 956, 852, 758, 730};

  //calls playTone to play the tone
  for (int i = 0; i < 13; i++)
  {
    if (names[i] == note)
    {
      playTone(tones[i], duration);
    }
  }
}

//function to blink the lights and play the song - acts as a mode
void playFrosty()
{
  for (int i = 0; i < length; i++)
  {
    //rests
    if (notes[i] == ' ')
    {
      delay(beats[i] * tempo); // rest
    }
    //notes
    else
    {
      playNote(notes[i], beats[i] * tempo);
    }

    //randomly lights up one LED in time with the music
    switcher = random(0,3);
    if (switcher == 0)
    {
      digitalWrite(ledBlu, LOW);
      digitalWrite(ledGrn, HIGH);
      digitalWrite(ledRed, LOW);
    }
    if (switcher == 1)
    {
      digitalWrite(ledBlu, LOW);
      digitalWrite(ledGrn, LOW);
      digitalWrite(ledRed, HIGH);
    }
    if (switcher == 2)
    {
      digitalWrite(ledBlu, HIGH);
      digitalWrite(ledGrn, LOW);
      digitalWrite(ledRed, LOW);
    }

    // pause between notes
    delay(tempo / 2);
  }
}

void setup()
{
  //pinmodes
  pinMode(ledBlu, OUTPUT);
  pinMode(ledGrn, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(speakerPin, OUTPUT);

  //eeprom and sleep setup
  ADCSRA &= ~(1<<ADEN);
  //if reset button is pressed...
  if (bit_is_set(MCUSR, EXTRF))
  {
    mode = EEPROM.read(1) % mode_count; //get current mode from eeprom
    EEPROM.write(1, (mode + 1) % mode_count); //store the next mode into eeprom
  }
  else
  {
    mode = 0;
    EEPROM.write(1, 1);
  }
}

void loop()
{
  //all the loop does is run the correct function depending on the current mode
  ModePtrs[mode]();
}
