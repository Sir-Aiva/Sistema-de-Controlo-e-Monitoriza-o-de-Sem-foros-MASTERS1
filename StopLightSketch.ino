#include <Arduino.h>
#include <time.h>
#include <stdlib.h>

// Interação com Portas

// Sinais nas Estradas
// Para melhorar a compreensão, usamos os sinais cardeais conforme os esquemas
const int signalNorth[] = {2, A5};
const int signalSouth[] = {A4, A3};
const int signalEast[] = {A2, A1};
const int signalWest[] = {7, 8};

//Sinal de Passagem de Peões
const int crossLed = 13;

//Botão de Passagem de Peões
const byte buttonPin = 3; // the pin that the pushbutton is attached to - interrupts

// Sensores do Tipo Hall
const int hallSensorNorth = 4;
const int hallSensorSouth = 5;
const int hallSensorEast = 9;
const int hallSensorWestStart = 10;
const int hallSensorWestEnd = 11;

// Buzzer dos Peões
const int buzzer = 12;

// Variáveis

// Tempo de espera entre sinais
int greenTimeInterval = 4000;
int yellowDelay = 5000;
int redTimeInterval = 5500;

//Passagem de Peões
int blinkDelay = 500;
int buzzerDelay = 500;
int walkRequest = 0;
int randomChoicePawns = 0;

// Sensores Hall
int hallInputDigital = 0;
int hallInputAnalogStart = 0;
int hallInputAnalogEnd = 0;

// Contadores Sensores Hall
int hallSensorCounterNorth;
int hallSensorCounterSouth;
int hallSensorCounterEast;
int hallSensorCounterWest;

char idCounterNorth = 'N';
char idCounterSouth = 'S';
char idCounterEast = 'E';
char idCounterWest = 'W';

// Speed Sensor Variables
int timerStart = 0;
int timerEnd = 0;
int timerTotal = 0;

// Distância entre Sensores (metros)
float speedDistance = 150;

// Limite de Velocidade entre os sensores (m/s)
// 14 m/s = +-50 Km/h
float speedLimit = 14;

// Valor Velocidade Média
float averageSpeed = 0;

// Variáveis First Stage
boolean firstStage = true;
int randomChoiceFirstStage = 0;

// Limite de carros em cada estrada
int carLimit = 4;

//XBEE
const byte frameStartByte = 0x7E;
const byte frameTypeTXrequest = 0x10;
const byte frameTypeRXpacket = 0x90;
const byte frameTypeATcommand = 0x08;
const byte frameTypeATresponse = 0x88;
const long destAddressHigh = 0x13A200;
const long destAddressLow = 0x40A099A4;
char DBcommand[] = "DB";

byte ATcounter=0; // for identifying current AT command frame
byte rssi = 0; // RSSI value of last received packet


void setup()
{
    // Start serial connection
    Serial.begin(9600);

    // Passagem de Peões
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(crossLed, OUTPUT);
    pinMode(buzzer, OUTPUT);
    attachInterrupt(1, pin_ISR, CHANGE);

    // Sensores de Hall
    pinMode(hallSensorNorth, INPUT_PULLUP);
    pinMode(hallSensorSouth, INPUT_PULLUP);
    pinMode(hallSensorEast, INPUT_PULLUP);
    pinMode(hallSensorWestStart, INPUT_PULLUP);
    pinMode(hallSensorWestEnd, INPUT_PULLUP);

    // Sinais de Trânsito
    for (int i = 0; i < 2; i++)
    {
        pinMode(signalNorth[i], OUTPUT);
        pinMode(signalSouth[i], OUTPUT);
        pinMode(signalEast[i], OUTPUT);
        pinMode(signalWest[i], OUTPUT);
    }
}

void loop()
{
    // Inicio ao sistema que executa a contagem de veiculos em cada estrada
    countSensorHall(hallSensorNorth, hallSensorCounterNorth, idCounterNorth);
    countSensorHall(hallSensorSouth, hallSensorCounterSouth, idCounterSouth);
    countSensorHall(hallSensorEast, hallSensorCounterEast, idCounterEast);

    // Inicio ao sistema que executa a monitorização de velocidade na estrada West
    speedSensorHall(hallSensorWestStart, hallSensorWestEnd, hallSensorCounterWest, idCounterWest);

    // Se é a primeira interação do sistema (após ser ligado ou reset), começa por esta função
    if (firstStage == true)
    {
        signalsStageOne();
        firstStage = false;
    }

    // Se for detetado que alguém pressionou o botão
    checkButtonPressed();

    char key = 'nul';
    if (Serial.available() >= 10) {  // Wait until we have a mouthful of data
      key = decodeAPIpacket();  // try to decode the received API frame from XBEE

      if (key == 'R') {
        // Ver isto
        // signalsStageOne();
        delay(10);
      }
    }

    // Estrada North
    if (hallSensorCounterNorth >= carLimit)
    {
        // Fechar todos os sinais abertos
        closeSignals();
        // Espera um tempo antes de passar para Verde
        delay(greenTimeInterval);
        // Abre o sinal da estrada a que diz respeito
        openSignals(signalNorth[0], signalNorth[1]);
        // Enviar 310
        formatTXAPIpacket(310);
        // Reset do contadores de Carros
        Serial.println("Reset do Contador North");
        hallSensorCounterNorth = 0;
    }

    // Se for detetado que alguém pressionou o botão
    checkButtonPressed();

    //Estrada South
    if (hallSensorCounterSouth >= carLimit)
    {
        // Fechar todos os sinais abertos
        closeSignals();
        // Espera um tempo antes de passar para Verde
        delay(greenTimeInterval);
        // Abre o sinal da estrada a que diz respeito
        openSignals(signalSouth[0], signalSouth[1]);
        // Enviar 320
        formatTXAPIpacket(320);
        // Reset do contadores de Carros
        Serial.println("Reset do Contador South");
        hallSensorCounterSouth = 0;
    }

    // Se for detetado que alguém pressionou o botão
    checkButtonPressed();

    // Estrada East
    if (hallSensorCounterEast >= carLimit)
    {
        // Fechar todos os sinais abertos
        closeSignals();
        // Espera um tempo antes de passar para Verde
        delay(greenTimeInterval);
        // Abre o sinal da estrada a que diz respeito
        openSignals(signalEast[0], signalEast[1]);
        // Enviar 330
        formatTXAPIpacket(330);
        // Reset do contadores de Carros
        Serial.println("Reset do Contador East");
        hallSensorCounterEast = 0;
    }

    // Se for detetado que alguém pressionou o botão
    checkButtonPressed();

    // Estrada West
    if (hallSensorCounterWest >= carLimit)
    {
        // Fechar todos os sinais abertos
        closeSignals();
        // Espera um tempo antes de passar para Verde
        delay(greenTimeInterval);
        // Abre o sinal da estrada a que diz respeito
        openSignals(signalWest[0], signalWest[1]);
        // Enviar 340
        formatTXAPIpacket(340);
        // Reset do contadores de Carros
        Serial.println("Reset do Contador West");
        hallSensorCounterWest = 0;
    }
}

// Numa primeira fase de arranque, queremos todos os sinais a vermelho excepto um (enquanto não temos veiculos guardados nos contadores)
// Isto por motivos de segurança
// O sinal que irá ficar a verde será escolhido aleatoriamente enquanto não houverem dados nos contadores
void signalsStageOne()
{
    Serial.println("Stage 1 - Inicio do Sistema");
    // Passar todos os sinais para Amarelo
    digitalWrite(signalNorth[0], HIGH); //Red 1
    digitalWrite(signalNorth[1], HIGH); //Green 1
    // Enviar 210
    formatTXAPIpacket(210);

    digitalWrite(signalSouth[0], HIGH);
    digitalWrite(signalSouth[1], HIGH);
    // Enviar 220
    formatTXAPIpacket(220);     
  
    digitalWrite(signalEast[0], HIGH);
    digitalWrite(signalEast[1], HIGH);
    // Enviar 230
    formatTXAPIpacket(230);

    digitalWrite(signalWest[0], HIGH);
    digitalWrite(signalWest[1], HIGH);
    // Enviar 240
    formatTXAPIpacket(240);

    // Wait x time declared in variable
    delay(yellowDelay);

    // Passar todos os sinais para Vermelho
    digitalWrite(signalNorth[0], HIGH); //Red 1
    digitalWrite(signalNorth[1], LOW);  //Green 0
    // Enviar 110
    formatTXAPIpacket(110);

    digitalWrite(signalSouth[0], HIGH);
    digitalWrite(signalSouth[1], LOW);
    // Enviar 120
    formatTXAPIpacket(120);

    digitalWrite(signalEast[0], HIGH);
    digitalWrite(signalEast[1], LOW);
    // Enviar 130
    formatTXAPIpacket(130);

    digitalWrite(signalWest[0], HIGH);
    digitalWrite(signalWest[1], LOW);
    // Enviar 140
    formatTXAPIpacket(140);

    delay(redTimeInterval);

    randomSeed(analogRead(A0));
    randomChoiceFirstStage = random(0, 3);
    //Serial.println("Número random = ");
    //Serial.println(randomChoiceFirstStage);

    switch (randomChoiceFirstStage)
    {
    case 0:
        // Estrada North
        Serial.println("Sinal Verde -> Estrada North");
        digitalWrite(signalNorth[0], LOW);  //Red 0
        digitalWrite(signalNorth[1], HIGH); //Green 1
        // Enviar 310
        formatTXAPIpacket(310);
        break;

    case 1:
        // Estrada South
        Serial.println("Sinal Verde -> Estrada South");
        digitalWrite(signalSouth[0], LOW);  //Red 0
        digitalWrite(signalSouth[1], HIGH); //Green 1
        // Enviar 320
        formatTXAPIpacket(320);
        break;

    case 2:
        // Estrada East
        Serial.println("Sinal Verde -> Estrada East");
        digitalWrite(signalEast[0], LOW);  //Red 0
        digitalWrite(signalEast[1], HIGH); //Green 1
        // Enviar 330
        formatTXAPIpacket(330);
        break;

    case 3:
        // Estrada West
        Serial.println("Sinal Verde -> Estrada West");
        digitalWrite(signalWest[0], LOW);  //Red 0
        digitalWrite(signalWest[1], HIGH); //Green 1
        // Enviar 340       
        formatTXAPIpacket(340); 
        break;
    default:
        Serial.println("Erro no Sistema!");
    }
}

// Função que passa os sinais para amarelos e depois vermelhos
void closeSignals()
{
    Serial.println("A fechar estradas...");

    // Sinais North
    // Verificar se North está a Verde
    int signalStateNorth = digitalRead(signalNorth[1]);

    // Verificar se South está a Verde
    int signalStateSouth = digitalRead(signalSouth[1]);

    // Verificar se East Está a Verde
    int signalStateEast = digitalRead(signalEast[1]);

    // Verificar se West Está a Verde
    int signalStateWest = digitalRead(signalWest[1]);

    // Se estiver algum destes sinais a Verde, transitar para Amarelo e depois Vermelho
    if (signalStateNorth == HIGH)
    {
        Serial.println("Fechar Sinais North");
        // Amarelo
        //delay(3000);
        digitalWrite(signalNorth[0], HIGH); //Red 1
        digitalWrite(signalNorth[1], HIGH); //Green 1
        //Enviar 210
        formatTXAPIpacket(210);

        // Wait x time declared in variable
        delay(yellowDelay);

        //Vermelho
        digitalWrite(signalNorth[0], HIGH); //Red 1
        digitalWrite(signalNorth[1], LOW);  //Green 0
        //Enviar 110
        formatTXAPIpacket(110);
    }
    else if (signalStateSouth == HIGH)
    {
        Serial.println("Fechar Sinais South");
        // Amarelo
        //delay(3000);
        digitalWrite(signalSouth[0], HIGH); //Red 1
        digitalWrite(signalSouth[1], HIGH); //Green 1
        //Enviar 220
        formatTXAPIpacket(220);

        // Wait x time declared in variable
        delay(yellowDelay);

        //Vermelho
        digitalWrite(signalSouth[0], HIGH); //Red 1
        digitalWrite(signalSouth[1], LOW);  //Green 0
        //Enviar 120
        formatTXAPIpacket(120);
    }
    else if (signalStateEast == HIGH)
    {
        Serial.println("Fechar Sinais East");
        // Amarelo
        //delay(3000);
        digitalWrite(signalEast[0], HIGH); //Red 1
        digitalWrite(signalEast[1], HIGH); //Green 1
        //Enviar 230
        formatTXAPIpacket(230);
        
        // Wait x time declared in variable
        delay(yellowDelay);

        //Vermelho
        digitalWrite(signalEast[0], HIGH); //Red 1
        digitalWrite(signalEast[1], LOW);  //Green 0
        //Enviar 130
        formatTXAPIpacket(130);
    }
    else if (signalStateWest == HIGH)
    {
        Serial.println("Fechar Sinais West");
        // Amarelo
        //delay(3000);
        digitalWrite(signalWest[0], HIGH); //Red 1
        digitalWrite(signalWest[1], HIGH); //Green 1
        //Enviar 240
        formatTXAPIpacket(240);

        // Wait x time declared in variable
        delay(yellowDelay);

        //Vermelho
        digitalWrite(signalWest[0], HIGH); //Red 1
        digitalWrite(signalWest[1], LOW);  //Green 0
        //Enviar 140
        formatTXAPIpacket(140);
    }
    else
    {
        Serial.println("Todas as estradas estão fechadas!");
    }
}

// Mudar sinais a verde
void openSignals(int signalOpenA, int signalOpenB)
{
    Serial.println("A abrir a estrada...");
    digitalWrite(signalOpenA, LOW);
    digitalWrite(signalOpenB, HIGH);
}

void btnPressed()
{
    Serial.println("Botão de Passagem Pressionado");
    //Enviar 170
    formatTXAPIpacket(170);

    closeSignals();
    Serial.println("Sinal dos Peões a ser Ativado");
    delay(2000);

    // Sinal do Peão pode começar a piscar e o Buzzer
    digitalWrite(crossLed, HIGH);
    tone(buzzer, 1000); // Send 1KHz sound signal...
    // Enviar 150
    formatTXAPIpacket(150);
    delay(1000);        // ...for 1 sec

    digitalWrite(crossLed, LOW);
    noTone(buzzer); // Stop sound...
    // Enviar 250
    formatTXAPIpacket(250);    
    delay(1000);    // ...for 1sec

    digitalWrite(crossLed, HIGH);
    tone(buzzer, 1000); // Send 1KHz sound signal...
    // Enviar 150
    formatTXAPIpacket(150);
    delay(1000);        // ...for 1 sec
    
    digitalWrite(crossLed, LOW);
    noTone(buzzer); // Stop sound...
    // Enviar 250
    formatTXAPIpacket(250);
    delay(1000);    // ...for 1sec

    digitalWrite(crossLed, HIGH);
    tone(buzzer, 1000); // Send 1KHz sound signal...
    // Enviar 150
    formatTXAPIpacket(150);
    delay(1000);        // ...for 1 sec

    digitalWrite(crossLed, LOW);
    noTone(buzzer); // Stop sound...
    // Enviar 250
    formatTXAPIpacket(250);
    delay(1000);    // ...for 1sec
    
    digitalWrite(crossLed, HIGH);
    tone(buzzer, 1000); // Send 1KHz sound signal...
    // Enviar 150
    formatTXAPIpacket(150);
    delay(1000);        // ...for 1 sec
    
    digitalWrite(crossLed, LOW);
    noTone(buzzer); // Stop sound...
    // Enviar 250
    formatTXAPIpacket(250);
    delay(1000);    // ...for 1sec

    walkRequest = 0;
    // Enviar 270
    formatTXAPIpacket(270);    

    delay(4000);

    randomSeed(analogRead(A0));
    //Serial.println("Analog Read");
    //Serial.println(analogRead(A0));
    randomChoicePawns = random(0, 3);
    //Serial.println("Número random = ");
    //Serial.println(randomChoicePawns);

    switch (randomChoicePawns)
    {
    case 0:
        // Estrada North
        Serial.println("Sinal Verde -> Estrada North");
        digitalWrite(signalNorth[0], LOW);  //Red 0
        digitalWrite(signalNorth[1], HIGH); //Green 1
        // Enviar 310
        formatTXAPIpacket(310);
        break;

    case 1:
        // Estrada South
        Serial.println("Sinal Verde -> Estrada South");
        digitalWrite(signalSouth[0], LOW);  //Red 0
        digitalWrite(signalSouth[1], HIGH); //Green 1
        // Enviar 320
        formatTXAPIpacket(320);
        break;

    case 2:
        // Estrada East
        Serial.println("Sinal Verde -> Estrada East");
        digitalWrite(signalEast[0], LOW);  //Red 0
        digitalWrite(signalEast[1], HIGH); //Green 1
        // Enviar 330
        formatTXAPIpacket(330);
        break;

    default:
        // Estrada West
        Serial.println("Sinal Verde -> Estrada West");
        digitalWrite(signalWest[0], LOW);  //Red 0
        digitalWrite(signalWest[1], HIGH); //Green 1
        // Enviar 340       
        formatTXAPIpacket(340); 
        break;
    }
}

void checkButtonPressed()
{
    // Se for detetado que alguém pressionou o botão, ele executa este código
    if (walkRequest == 1)
    {
        btnPressed();
    }
}
// Muda a variável para 1 de modo a que seja captada no loop principal
void pin_ISR()
{
    walkRequest = 1;
}

void chooseRoad() {
  randomSeed(analogRead(A0));
    //Serial.println("Analog Read");
    //Serial.println(analogRead(A0));
  randomChoicePawns = random(0, 3);
    //Serial.println("Número random = ");
    //Serial.println(randomChoicePawns);
    switch (randomChoicePawns)
    {
    case 0:
        // Estrada North
        Serial.println("Sinal Verde -> Estrada North");
        digitalWrite(signalNorth[0], LOW);  //Red 0
        digitalWrite(signalNorth[1], HIGH); //Green 1
        // Enviar 310
        formatTXAPIpacket(310);
        break;

    case 1:
        // Estrada South
        Serial.println("Sinal Verde -> Estrada South");
        digitalWrite(signalSouth[0], LOW);  //Red 0
        digitalWrite(signalSouth[1], HIGH); //Green 1
        // Enviar 320
        formatTXAPIpacket(320);
        break;

    case 2:
        // Estrada East
        Serial.println("Sinal Verde -> Estrada East");
        digitalWrite(signalEast[0], LOW);  //Red 0
        digitalWrite(signalEast[1], HIGH); //Green 1
        // Enviar 330
        formatTXAPIpacket(330);
        break;

    default:
        // Estrada West
        Serial.println("Sinal Verde -> Estrada West");
        digitalWrite(signalWest[0], LOW);  //Red 0
        digitalWrite(signalWest[1], HIGH); //Green 1
        // Enviar 340       
        formatTXAPIpacket(340); 
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Digital
void countSensorHall(int hallSensor, int &hallSensorCounter, char idCounter)
{
    hallInputDigital = digitalRead(hallSensor);
    if (hallInputDigital == LOW)
    {
        ++hallSensorCounter;
        Serial.print("Contador: ");
        Serial.println(idCounter);
        Serial.print("Número de Veículos: ");
        Serial.println(hallSensorCounter);

        if (idCounter == 'N') {
          // Enviar 160
          formatTXAPIpacket(160);        
        } else if (idCounter == 'S') {
          //Enviar 260
          formatTXAPIpacket(260);
        } else if (idCounter == 'E') {
          //Enviar 360
          formatTXAPIpacket(360);
        } else {
          //Enviar 460
          formatTXAPIpacket(460);
        }
    }
    // mudar para menos?
    delay(150);
}

// Analog
// Sensores D e E
void speedSensorHall(int hallSensorStart, int hallSensorEnd, int &hallSensorCounter, char idCounter)
{
    hallInputAnalogStart = digitalRead(hallSensorStart);
    hallInputAnalogEnd = digitalRead(hallSensorEnd);
    if (hallInputAnalogStart == HIGH)
    {
        Serial.println("Disparou o Sensor West Start");
        timerStart = 0;
        timerStart = millis();
        ++hallSensorCounter;
        // Código para enviar
        formatTXAPIpacket(460);
        Serial.print("Contador: ");
        Serial.println(idCounter);
        Serial.print("Número de Veículos: ");
        Serial.println(hallSensorCounter);
    }

    if (hallInputAnalogEnd == HIGH)
    {
        Serial.println("Disparou o Sensor West End");
        timerEnd = 0;
        timerEnd = millis();
        timerTotal = timerEnd - timerStart;
        timerStart = 0;
        timerEnd = 0;
    }

    delay(1000);

    if (timerTotal > 1000)
    {
        Serial.println("Timer acima de 1000");
        Serial.println("Timer (ms): ");
        Serial.println(timerTotal);

        averageSpeed = (speedDistance / (timerTotal * 0.001));
        Serial.println("Average Speed (m/s): ");
        Serial.println(averageSpeed);

        // Enviar Average Speed
        int averageSpeedXbee = trunc(averageSpeed);    
        averageSpeedXbee = averageSpeedXbee * 100;   
        averageSpeedXbee = averageSpeedXbee + 80;        
        formatTXAPIpacket(averageSpeedXbee);

        if (averageSpeed > speedLimit)
        {
            Serial.println("Foi Ultrapassado o Limite de Velocidade!");

            // Verificar se West Está a Verde
            int signalStateWest = digitalRead(signalWest[1]);

            if (signalStateWest == HIGH)
            {
                Serial.println("Fechar Sinais West");
                // Amarelo
                //delay(3000);
                digitalWrite(signalWest[0], HIGH); //Red 1
                digitalWrite(signalWest[1], HIGH); //Green 1
                //Enviar 240
                formatTXAPIpacket(240);

                // Wait x time declared in variable
                delay(yellowDelay);

                //Vermelho
                digitalWrite(signalWest[0], HIGH); //Red 1
                digitalWrite(signalWest[1], LOW);  //Green 0
                //Enviar 140
                formatTXAPIpacket(140);

                delay(2500);
                chooseRoad();
            }                     
        }
    }
    timerTotal = 0;
}



//XBEE
void formatATcommandAPI(char* command) {
  // Format and transmit a AT Command API frame
  long sum = 0;  // Accumulate the checksum

  ATcounter += 1;  // increment frame counter

  // API frame Start Delimiter
  Serial.write(frameStartByte);

  // Length - High and low parts of the frame length (Number of bytes between the length and the checksum)
  Serial.write(0x00);
  Serial.write(0x04);

  // Frame Type - Indicate this frame contains a AT Command
  Serial.write(frameTypeATcommand); 
  sum += frameTypeATcommand;

  // Frame ID – cannot be zero for receiving a reply
  Serial.write(ATcounter);
  sum += ATcounter;

  // AT Command
  Serial.write(command[0]);
  Serial.write(command[1]);
  sum += command[0];
  sum += command[1];

  // Parameter Value for the Command - Optional

  // Checksum = 0xFF - the 8 bit sum of bytes from offset 3 (Frame Type) to this byte.
  Serial.write(0xFF - (sum & 0xFF));

  delay(10);  // Pause to let the microcontroller settle down if needed
}


char decodeAPIpacket(void) {
  // Function for decoding the received API frame from XBEE
  char rxbyte = 'nul';
  byte frametype;

  while (Serial.read() != frameStartByte) {
    if (Serial.available() == 0)
      return rxbyte;  // No API frame present. Return 'nul'
  }

  // Skip over length field bytes in the API frame
  Serial.read();  // MSB
  Serial.read();  // LSB

  // Read received frame type
  frametype = Serial.read();

  if (frametype == frameTypeRXpacket) {
    // Zigbee Receive Packet API FRame

    // Skip over the bytes in the API frame we don't care about (addresses, receive options)
    for (int i = 0; i < 11; i++) {
      Serial.read();
    }
    // The next byte is the key pressed and sent from the remote XBEE
    rxbyte = Serial.read();

    Serial.read();  // Read  the last byte (Checksum) but don't store it

    formatATcommandAPI("DB");  // query the RSSI of the last received packet

  } else if (frametype == frameTypeATresponse) {
     //AT Command Response API frame

     //Skip over the bytes in the API frame we don't care about (addresses, receive options)    
    for (int i = 0; i < 4; i++) {
      Serial.read();
    }
    rssi = Serial.read();
 
    Serial.read(); // Read the last byte (Checksum) but don't store it
  }
  return rxbyte;
}

void formatTXAPIpacket(int value) {
  // Format and transmit a Transmit Request API frame

  long sum = 0;  // Accumulate the checksum

  // API frame Start Delimiter
  Serial.write(frameStartByte);

  // Length - High and low parts of the frame length (Number of bytes between the length and the checksum)
  Serial.write(0x00);
  Serial.write(0x11);

  // Frame Type - Indicate this frame contains a Transmit Request
  Serial.write(frameTypeTXrequest);
  sum += frameTypeTXrequest;

  // Frame ID - set to zero for no reply
  Serial.write(0x00);
  sum += 0x00;

  // 64-bit Destination Address - The following 8 bytes indicate the 64 bit address of the recipient (high and low parts).
  // Use 0xFFFF to broadcast to all nodes.
  Serial.write((destAddressHigh >> 24) & 0xFF);
  Serial.write((destAddressHigh >> 16) & 0xFF);
  Serial.write((destAddressHigh >> 8) & 0xFF);
  Serial.write((destAddressHigh)&0xFF);
  sum += ((destAddressHigh >> 24) & 0xFF);
  sum += ((destAddressHigh >> 16) & 0xFF);
  sum += ((destAddressHigh >> 8) & 0xFF);
  sum += (destAddressHigh & 0xFF);
  Serial.write((destAddressLow >> 24) & 0xFF);
  Serial.write((destAddressLow >> 16) & 0xFF);
  Serial.write((destAddressLow >> 8) & 0xFF);
  Serial.write(destAddressLow & 0xFF);
  sum += ((destAddressLow >> 24) & 0xFF);
  sum += ((destAddressLow >> 16) & 0xFF);
  sum += ((destAddressLow >> 8) & 0xFF);
  sum += (destAddressLow & 0xFF);

  // 16-bit Destination Network Address - The following 2 bytes indicate the 16-bit address of the recipient.
  // Use 0xFFFE if the address is unknown.
  Serial.write(0xFF);
  Serial.write(0xFE);
  sum += 0xFF + 0xFE;

  // Broadcast Radius - when set to 0, the broadcast radius will be set to the maximum hops value
  Serial.write(0x00);
  sum += 0x00;

  // Options
  // 0x01 - Disable retries and route repair
  // 0x20 - Enable APS encryption (if EE=1)
  // 0x40 - Use the extended transmission timeout
  Serial.write(0x00);
  sum += 0x00;

  // RF Data
  Serial.write((value >> 8) & 0xFF);  // ADC temperature reading (high byte)
  Serial.write(value & 0xFF);         // ADC temperature reading (low byte)
  sum += ((value >> 8) & 0xFF) + (value & 0xFF);
  Serial.write(rssi); // RSSI reading
  sum += rssi;

  // Checksum = 0xFF - the 8 bit sum of bytes from offset 3 (Frame Type) to this byte.
  Serial.write(0xFF - (sum & 0xFF));

  delay(10);  // Pause to let the microcontroller settle down if needed
}


