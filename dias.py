import RPi.GPIO as GPIO,time
import logging
import pymsgbox
from time import sleep
from datetime import datetime
from sh import gphoto2 as gp
import signal, os,subprocess
import sys

#Hardware
Druecker=8		#GPIO Pin 

#Einzelauslösung oder Serienbildfunktion
Serie=1			#0: Einzelbild; sonst Serienbildfunktion

#Zeiten
Wait_t=4		#Gesamtzeit pro Bild
Wait_1=2		#Wartezeit von Auslösen der Kamera bis Transport des Projektors
Press=0.3		#Dauer des Drückens für Transport des Projektors 

Denkp=5			#nur bei Serienbild: Anzahl Wiederholungen am Anfang
suchStr=b'Nehme Bild'

triggerCommand = ["--trigger-capture"]

logging.basicConfig(filename='dias.log',level=logging.DEBUG,format='%(asctime)s %(message)s')
logging.info('Start dias.py')
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(Druecker, GPIO.OUT)
GPIO.output(Druecker,GPIO.HIGH)

p = subprocess.Popen(['ps', '-A'], stdout=subprocess.PIPE)
out, err = p.communicate()

# Search for the process we want to kill
for line in out.splitlines():
    if b'gvfsd-gphoto2' in line:
        # Kill that process!
        pid = int(line.split(None,1)[0])
        logging.info(f"PID to kill: {pid}")
        os.kill(pid, signal.SIGKILL)


logging.info('nach killGphoto2Process')
while 1:
    ausw=pymsgbox.confirm('Anzahl Dias pro Schiene', 'Schienentyp',['?','1','36','50','60','80','100','Abbruch'])
    try:
        AnzahlDias=int(ausw)
    except ValueError:
        if ausw=="?":
            ausw=input("Anzahl Dias eingeben")
            try:
                AnzahlDias=int(ausw)
                print(AnzahlDias)
            except ValueError:
                AnzahlDias=-1
        else:
            AnzahlDias=-1
    if AnzahlDias <0:
        break
    logging.info(f"Anzahl Dias: {AnzahlDias}")
    #etime.sleep(10)
    if AnzahlDias ==1:
        gp(triggerCommand)
	elif Serie==0:
		Wait_2=Wait_t-Wait_1
		for BildNr in range(AnzahlDias):
			gp(triggerCommand)
			time.sleep(Wait_1)
			GPIO.output(Druecker,GPIO.LOW)
			time.sleep(Press)
			GPIO.output(Druecker,GPIO.HIGH)
			time.sleep(Wait_2)
    else:
#        GPIO.output(Druecker,GPIO.LOW)
#        time.sleep(0.3)
#        GPIO.output(Druecker,GPIO.HIGH)

        proc=subprocess.Popen(['gphoto2','-I',str(Wait_t),'-F',str(AnzahlDias+Denkp-1),'--set-config','viewfinder=1',
                            '--set-config','autofocus=off','--capture-image'],stdout=subprocess.PIPE)


        logging.info(f"nach subprocess")
        BildNr=0

        while True:
            line=proc.stdout.readline()
            if not line:
                break
            logging.info(line.rstrip())
#            if b'Nehme Bild' in line:
            if suchStr in line:
                BildNr+=1
                if BildNr<Denkp:
                    logging.info(f"kein Transport")
                else:
                    logging.info(f"in 2s Dia wechseln")
                    time.sleep(Wait_1)
                    GPIO.output(Druecker,GPIO.LOW)
                    time.sleep(Press)
                    GPIO.output(Druecker,GPIO.HIGH)
        logging.info(f"Ende")
