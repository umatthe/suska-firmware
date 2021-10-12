#define SHOWZEIT
/* Puzzleberechnung, Version 2, 22. Oktober 1996 */
/* Sucht alle Kombinationen. Die Lîsungen zeigen die Reihenfolge
   der Formen, begonnen in einer Ecke und dann erst in x- dann in
   y- und schliesslich in z-Richtung eingefÅgt. */

/* Laufzeit:
   Pentium 100MHz: DOS 441s, Win95 455s */

/* Mit SPEZIFISCH sind Stellen gekennzeichnet, die bei einem
   anderen Puzzle evtl. geaendert werden muessen. */

/* Speicherplatz Optimierung
 * Original Klaus: #define kint int
 */
//#define kint int
//#define kint short
#define kint char

#define _HAS_STDLIB_
//#define _HAS_FILE_
//#define _HAS_FLOAT_
//#define _ARDUINO_
#define _YH_AVR_
#define _SUSKA_AVR_
//#define circuit ZPUino_Vanilla
//#define _ZPUino_Timer_
#ifndef _ARDUINO_
#include <stdio.h>
#ifdef _HAS_STDLIB_
#include <stdlib.h>
#include <time.h>
#endif
#else
#ifdef _ZPUino_Timer_
#include <Timer.h>
long ti=1L;
#endif
#endif
#ifdef _YH_AVR_
#include <stdio.h>
#include "../timer/tick.h"
#ifdef SUSKA_BF
#include "../suska-iii/timer_overflow.h"
#endif
#endif

#define GESAMT_X   5    /* Breite des gesamten Puzzles, SPEZIFISCH */
#define GESAMT_Y   5    /* Hoehe des gesamten Puzzles, SPEZIFISCH */
#define GESAMT_Z   5    /* Tiefe des gesamten Puzzles, SPEZIFISCH */
#define NR_STEINE  3    /* Anzahl verschiedener Steine, SPEZIFISCH */
#define NR_TEILE  17    /* Anzahl Teile pro Puzzle, SPEZIFISCH */
#define NR_FORMEN 10    /* Anzahl verschiedener Formen, SPEZIFISCH */
                        /* eine Form ist ein Stein in einer
                           bestimmten Lage */

#define TEIL_X     0    /* Indizes fuer Array size */
#define TEIL_Y     1
#define TEIL_Z     2
#define TEIL_NR    3

#define FLG_FORM   0    /* Indizes fuer Array folge */
#define FLG_X      1
#define FLG_Y      2
#define FLG_Z      3

#define FREI      99    /* freies Feld in matrix oder folge */

#define FALSE      0
#define TRUE       1
#define BACKSPACE  8    /* fuer Ausgabe Lebenszeichen */

void init(void);            /* Variablen vorbesetzen */
kint check(void);            /* pruefen, ob Form passt */
void eintragen(void);       /* in Matrix und Folge eintragen */
void austragen(void);       /* aus Matrix und folge loeschen */
void suche_frei(void);      /* freien Platz suchen */
void zurueck(void);         /* Rueckschritt */
void loesche_matrix(void);  /* Matrix komplett loeschen */
void loesche_folge(void);   /* Folge komplett loeschen */
void output(void);          /* Ausgabe */
void puzzle(int runden);

#ifdef _HAS_FILE_ 
FILE *filename;
#endif

int frei_x, frei_y, frei_z; /* naechstes freies Feld */

kint size[NR_FORMEN][4] =    /* Abmessungen der Teile, SPEZIFISCH */
  {{4, 2, 1, 0},            /* Form 0 */
   {4, 1, 2, 0},            /* Form 1 */
   {2, 4, 1, 0},
   {2, 1, 4, 0},
   {1, 4, 2, 0},
   {1, 2, 4, 0},
   {3, 2, 2, 1},
   {2, 3, 2, 1},
   {2, 2, 3, 1},
   {1, 1, 1, 2}};           /* Form 9 */
/* Der jeweils 4. Wert ist die Teilenummer. Dieser Wert
   wird zum Zaehlen der Teile benoetigt. */

/*  x/y-Ansicht der Formen (Tiefe z nicht sichtbar):

              22      4
              22      4          77
  0000        22      4  5  666  77  88
  0000  1111  22  33  4  5  666  77  88  9  */

kint anzahl[NR_STEINE] =     /* Anz. der Steine, SPEZIFISCH */
          {6, 6, 5};

kint nr[NR_STEINE];          /* Anzahl-Zaehler */
kint matrix[GESAMT_X]        /* speichert Puzzle */
           [GESAMT_Y]
           [GESAMT_Z];
int stelle;                 /* momentane Position fuer check */
int form;                   /* momentane Form fuer check */
int fertig;                 /* wird 1, wenn komplett fertig */
long versuche;              /* zaehlt Versuche */
int kombi;                  /* zaehlt gefundene Kombinationen */
kint folge[NR_TEILE + 1][4]; /* speichert Form und Koordinaten */
int voll;                   /* TRUE, wenn Loesung gefunden */
int letzte_form;            /* entspricht NR_FORMEN - 1 */

char zeichen[6] = "|/-\\ "; /* fuer Ausgabe Lebenszeichen */
int zeichen_nr;

long zeit, startzeit;       /* fuer Zeitmessung
                               (time_t entspricht long) */
#ifdef SHOWZEIT
long zzeit;
#endif

/*************************************************************/

kint runden=0;
kint runde;
#ifndef _SUSKA_AVR_
#ifndef _ARDUINO_
int main(int argc, char *argv[])
#else
int pmain(int argv)
#endif
{
	runden=1;
#ifndef _ARDUINO_
#ifdef _HAS_STDLIB_
	if(argc == 2)
	{
		runden=atoi(argv[1]);
	}
#else
	printf("no Parameter evaluation\n");
#endif
#else
	runden=argv;
#endif
	printf("%d Runden\n",runden);

#ifndef _ARDUINO_
	startzeit = time(0);
#else
#ifdef _ZPUino_Timer_
	startzeit = ti/10;
#elif defined _YH_AVR_
	startzeit = tick/100;
#else
	startzeit = mirco()/1000000;
#endif
#endif
	puzzle(runden);
#ifndef _ARDUINO_
	zeit = time(0);
#else
#ifdef _ZPUino_Timer_
	zeit = ti/10;
#elif defined _YH_AVR_
	zeit =tick/100; 
#else
	zeit = mirco()/1000000;
#endif
#endif
	printf("%c%c", zeichen[4], BACKSPACE);  /* Symbol loeschen */
	printf("\n\nLaufzeit: %ld Sekunden.\n\n", zeit - startzeit);
#ifdef _HAS_FLOAT_
	printf("\n\nLaufzeit: %6.3f Sekunden (pro Runde).\n\n",((float)(zeit - startzeit))/runden);
#else
	printf("\n\nLaufzeit: %ld.%ld Sekunden (pro Runde).\n\n",(zeit-startzeit)/runden,((zeit-startzeit)*10)/runden-((zeit-startzeit)/runden)*10);
#endif
#ifdef _HAS_FILE_
	printf("Ergebnis in ASCII-Datei \"PUZZLE.DAT\"!\n\n");
#endif
	printf("Versuche: %ld\n", versuche);
	return(0);
}
#else
void shell_puzzle(uint8_t *para)
{
#ifdef SUSKA_BF
       // PS2-Keyboad and Tick use the same Timer 0
       // Init for tick to measure time
       t0_off();
       tick_init(TICK10MS);
#endif
        sscanf(para,"%d",&runden);
	startzeit = tick/100;
        puzzle(runden);
	zeit =tick/100; 
	printf("%c%c", zeichen[4], BACKSPACE);  /* Symbol loeschen */
	printf("\n\nLaufzeit: %ld Sekunden.\n\n", zeit - startzeit);
	printf("\n\nLaufzeit: %ld.%ld Sekunden (pro Runde).\n\n",(zeit-startzeit)/runden,((zeit-startzeit)*10)/runden-((zeit-startzeit)/runden)*10);
	printf("Versuche: %10ld\n", versuche);
#ifdef SUSKA_BF
       // PS2-Keyboad and Tick use the same Timer 0
       // Puzzle done - ReInit for ps2
        timer_init();
#endif
}
#endif

void puzzle(int runden)
{
  for(runde=0;runde<runden;runde++)
  {
  printf("Kombinationen werden gesucht...\n\n");
#ifdef _HAS_FILE_
  filename = fopen("PUZZLE.DAT", "wt");
#endif
  init();
  while (fertig == FALSE)
    {
      if (check() == TRUE)     /* Form passt */
        {
          eintragen();
	  suche_frei();
          if (voll == TRUE)    /* Seite voll */
            {
              voll = FALSE;
              output();
              kombi++;         /* zaehlt Loesungen */
              zurueck();
            }
          else                 /* Seite nicht voll */
            {
              /*  SPEZIFISCH: Probiere immer erst Form 0,
                  wenn eine Art von Steinen ver-
                  braucht ist, muss die erste Form des
                  naechsten Steins verwendet werden. */
              if (nr[0] < anzahl[0])  /* noch Steine vorhanden */
                {
                  form = 0;  /* wieder mit Form 0 beginnen */
                  stelle++;
                }
              else if (nr[1] < anzahl[1])
                {
                  form = 6;  /* keine Steine Form 0 mehr */
                  stelle++;
                }
              else if (nr[2] < anzahl[2])
                {
                  form = 9;  /* auch keine Form 6 mehr */
                  stelle++;
                }
              else
                {
                  zurueck(); /* auch keine Form 9 mehr */
                }
            }
        }
      else    /* Form passt nicht, Form kann nicht 9 sein */
        {
          /*  SPEZIFISCH: Dieser Teil muss evtl. vollstaendig
              geaendert werden.
              Funktion: Auswaehlen der naechsten Form, von deren
              Steinen noch welche vorhanden sind. Falls keine
              Steine mehr vorhanden sind, gehe 1 Schritt zurueck. */
          form++;
          if (nr[size[form][TEIL_NR]] == anzahl[size[form][TEIL_NR]])
            {
              if (form <= 8)
                {
                  form = 9;
                }
              if ((form == 9) && (nr[2] == anzahl[2]))
                {
                  stelle--;    /* hier (stelle) passt NICHTS! */
                  zurueck();
                }
            }
        }
    }
#ifdef _HAS_FILE_
  fclose(filename);
#endif
  }
}

/**************************************************************/

void init(void)
{
  voll = FALSE;
  fertig = FALSE;
  form = 0;        /* mit erster Form beginnen */
  stelle = 0;      /* zaehlt Steine pro Seite */
  kombi = 0;
  versuche = 0L;
  nr[0] = 0;
  nr[1] = 0;
  nr[2] = 0;
  loesche_matrix();
  loesche_folge();
  frei_x = 0;
  frei_y = 0;
  frei_z = 0;
  zeichen_nr = 0;
  letzte_form = NR_FORMEN - 1;
}

/**************************************************************/

kint check(void)
{
  int x = 0, y = 0, z = 0, ok = TRUE;
  versuche++;
  if (((frei_x + size[form][TEIL_X]) <= GESAMT_X) &&
      ((frei_y + size[form][TEIL_Y]) <= GESAMT_Y) &&
      ((frei_z + size[form][TEIL_Z]) <= GESAMT_Z))
    { /* Form passt schon mal in den Puzzlekoerper */
      while ((z < size[form][TEIL_Z]) && (ok == TRUE))
        {
          while ((y < size[form][TEIL_Y]) && (ok == TRUE))
	    {
              while ((x < size[form][TEIL_X]) && (ok == TRUE))
                {
                  if (matrix[frei_x + x]
                            [frei_y + y]
                            [frei_z + z] != FREI)
                    {  /* matrix-Feld bereits belegt */
                      ok = FALSE;
                    }
                  x++;
                }
              x = 0;
              y++;
            }
          x = 0;
          y = 0;
          z++;
        }
    }
  else  /* Form verlaesst die Seiten des Puzzles */
    {
      ok = FALSE;
    }
  return(ok);
}

/**************************************************************/

void eintragen(void)
{
  kint x, y, z;
  for (z = 0; z < size[form][TEIL_Z]; z++)
    {
      for (y = 0; y < size[form][TEIL_Y]; y++)
        {
	  for (x = 0; x < size[form][TEIL_X]; x++)
            {
              matrix[frei_x + x][frei_y + y][frei_z + z] = form;
            }
        }
    }
  folge[stelle][FLG_FORM] = form;
  folge[stelle][FLG_X] = frei_x;
  folge[stelle][FLG_Y] = frei_y;
  folge[stelle][FLG_Z] = frei_z;
  nr[size[form][TEIL_NR]]++;        /* Zaehler inkrementieren */
}

/**************************************************************/

void austragen(void)
{
  kint x, y, z, xx, yy, zz;
  xx = folge[stelle][FLG_X];
  yy = folge[stelle][FLG_Y];
  zz = folge[stelle][FLG_Z];
  for (z = 0; z < size[form][TEIL_Z]; z++)
    {
      for (y = 0; y < size[form][TEIL_Y]; y++)
        {
          for (x = 0; x < size[form][TEIL_X]; x++)
            {
              matrix[xx + x][yy + y][zz + z] = FREI;  /* loeschen */
            }
        }
    }
  folge[stelle][FLG_FORM] = FREI;
  folge[stelle][FLG_X] = FREI;
  folge[stelle][FLG_Y] = FREI;
  folge[stelle][FLG_Z] = FREI;
  nr[size[form][TEIL_NR]]--;    /* Zaehler dekrementieren */
}

/**************************************************************/

void suche_frei(void)
{
  while ((frei_z < GESAMT_Z) &&
         (matrix[frei_x][frei_y][frei_z] != FREI))
    {
      frei_x++;
      if (frei_x == GESAMT_X)
        {
          frei_y++;
          frei_x = 0;
          if (frei_y == GESAMT_Y)
            {
              frei_z++;
              frei_y = 0;
            }
        }
    }
  if (frei_z == GESAMT_Z)  /* keine freien Felder mehr */
    {
      frei_z--;
      voll = TRUE;         /* Loesung gefunden! */
    }
}

/**************************************************************/

void zurueck(void)
{
  kint ok = FALSE, plus_1, alt_x, alt_y, alt_z;
  if ((versuche % 2500) == 0)
    {
      zeichen_nr++;
      if (zeichen_nr == 4)
        {
          zeichen_nr = 0;
        }
      printf("%c%c", zeichen[zeichen_nr], BACKSPACE);
    }
  while ((ok == FALSE) && (fertig == FALSE))
    {
      plus_1 = TRUE;
      form = folge[stelle][FLG_FORM];
      while ((fertig == FALSE) &&
             ((form == letzte_form) || (plus_1 == TRUE)))
        {
          alt_x = folge[stelle][FLG_X];    /* freie Pos. merken */
          alt_y = folge[stelle][FLG_Y];
          alt_z = folge[stelle][FLG_Z];
          austragen();
          if (form == letzte_form)
            {
              stelle--;
              if (stelle == -1) /* weiter zurueck geht es nicht */
                {
                  fertig = TRUE;
                }
              else
                {
                  form = folge[stelle][FLG_FORM];
                }
            }
          else
            {
              plus_1 = FALSE;
            }
	}
      if (fertig == FALSE)
        {
          form++;
          if (nr[size[form][TEIL_NR]] < anzahl[size[form][TEIL_NR]])
            {
              ok = TRUE;
            }
          else
            {
              stelle--;
            }
        }
    }
  if (fertig == FALSE)
    {
      frei_x = alt_x;
      frei_y = alt_y;
      frei_z = alt_z;
    }
}

/**************************************************************/

void loesche_matrix(void)
{
  int x, y, z;
  for (x = 0; x < GESAMT_X; x++)
    {
      for (y = 0; y < GESAMT_Y; y++)
        {
          for (z = 0; z < GESAMT_Z; z++)
            {
              matrix[x][y][z] = FREI;
            }
	}
    }
}

/**************************************************************/

void loesche_folge(void)
{
  int i;
  for (i = 0; i < (NR_TEILE + 1); i++)
    {
      folge[i][FLG_FORM] = FREI;
      folge[i][FLG_X] = FREI;
      folge[i][FLG_Y] = FREI;
      folge[i][FLG_Z] = FREI;
    }
}

/**************************************************************/

void output(void)
{
  int i = 0;
#ifdef _HAS_FILE_
  fprintf(filename, "Loesung %d: ", kombi);
#endif
  printf("  Loesung %d: ", kombi);
  while (folge[i][FLG_FORM] != FREI)
    {
#ifdef _HAS_FILE_
      fprintf(filename, "%d ", folge[i][FLG_FORM]);
#endif
      printf("%d ", folge[i][FLG_FORM]);
      i++;
    }
#ifdef _HAS_FILE_
  fprintf(filename, "   (nach %9ld Versuchen)\n", versuche);
#endif

#ifdef SHOWZEIT
  zzeit =tick/100; 
  printf(" %9ld Vers. %ld s\n", versuche,zzeit-startzeit);
#else
  printf("  (nach %9ld Versuchen)\n", versuche);
#endif

}

/**************************************************************/

#ifdef _ARDUINO_
void setup() 
{
#ifndef _YH_AVR_ 
	//Delay for 3 seconds to prevent detection as a serial mouse.
	delay(3000);

	//Setup Serial port and send out Title
	Serial.begin(9600); 
#endif
#ifdef _ZPUino_Timer_
	Timers.begin();
	int r = Timers.periodicHz(1, timer, 0, 1);
	if (r<0) {
		printf("Fatal error!\n");
	}   
	ti=0;
#endif
}

#ifdef _ZPUino_Timer_
bool timer(void*)
{
	//1Hz Timer

	ti=ti+1L;
	return true;
}
#endif

void loop() 
{
#ifndef _YH_AVR_
	delay(1000);
#endif
	printf("Arduino Puzzle start\n\n");
	pmain(1);
	printf("Arduino Puzzle done\n\n");
	while(1);
}
#endif

