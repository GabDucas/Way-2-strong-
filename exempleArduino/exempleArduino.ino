

double angle;
double angle2;
double angle3;

double increment;
double nbValue;

String commande;

void setup(){

  pinMode(4, HIGH);
  Serial.begin(9600);                        //  setup serial
  angle = 0.0;
  angle2 = 0.0;
  angle3 = 0.0;
  increment = 0.0;
  nbValue = 500;
  randomSeed(analogRead(A0));
}

void loop(){
  
  int randomInt = random(-(nbValue/2), nbValue/2);
  float increment = randomInt * PI/(8*nbValue);
  angle = angle + increment;


  randomInt = random(-(nbValue/2), nbValue/2);
  increment = randomInt * PI/(8*nbValue);
  angle2 = angle2 + increment;

  randomInt = random(-(nbValue/2), nbValue/2);
  increment = randomInt * PI/(8*nbValue);
  angle3 = angle3 + increment;

  Serial.print(angle);
  Serial.print(",");

  Serial.print(angle2);
  Serial.print(",");

  Serial.println(angle3);

  if(Serial.available())
    {
        commande = Serial.readStringUntil('\n');
        commande.trim();
        if(commande.equals("ON"))
          angle2 = 10000000;
    }

  delay(200);
  } 