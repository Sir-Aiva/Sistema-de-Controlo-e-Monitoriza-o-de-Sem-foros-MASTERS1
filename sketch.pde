import processing.serial.*;

// Quando eu fiz este código, foi uma atividade feita entre mim e Deus
// Agora é só entre Deus

// Boa sorte para quem tentar decifrar isto
Serial myPort;

// Background Image
PImage img;

/* Communications and sensors related variables */
int portIndex = 1; // WARNING: Set this to the port connected to XBEE Explorer (0 is the first port)
byte frameStartByte = 0x7E;
byte frameTypeTXrequest = 0x10;
int destAddressHigh = 0x13A200;
int destAddressLow = 0x414F7F10;
int val; // Data received from the serial port
int rssi=100;

// Contadores Número de Carros
int contNorth = 0;
int contSouth = 0;
int contEast = 0;
int contWest = 0;

// Velocidade Média
long speedWest;

/* drawing related variables */
PFont font; // create a font for display
int fontSize = 12;

// Classe dos Semáforos
stoplightNorth northStoplight;
stoplightSouth southStoplight;
stoplightEast eastStoplight;
stoplightWest westStoplight;

// Classe do Semáforo de Peão - estrada Norte
stoplightPawns stoplightPawnsNorthA;
stoplightPawns stoplightPawnsNorthB;

// Botão pressionado
boolean btnPressed = false;

void setup() {
  // Mesmo tamanho que a imagem do background
  size(975, 934); // screen size
  smooth(); // anti-aliasing for graphic display
  font.list();
  font = createFont("Arial.normal", fontSize);
  textFont(font);

  println(Serial.list()); // print the list of all the ports
  println(" Connecting to -> " + Serial.list()[portIndex]);
  myPort = new Serial(this, Serial.list()[portIndex], 9600);
  myPort.clear(); // clear buffer

  //Background
  img = loadImage("Road2.jpeg");
  background(img);

  //Semáforos
  println("GUI");
  northStoplight = new stoplightNorth(380, 290);
  northStoplight.display();

  southStoplight = new stoplightSouth(600, 550);
  southStoplight.display();

  eastStoplight = new stoplightEast(600, 335);
  eastStoplight.display();

  westStoplight = new stoplightWest(335, 550);
  westStoplight.display();

  stoplightPawnsNorthA = new stoplightPawns(380, 245);
  stoplightPawnsNorthA.display();

  stoplightPawnsNorthB = new stoplightPawns(600, 210);
  stoplightPawnsNorthB.display();
}

void draw() {
  // Função que processa o valor recebido do xbee
  if (myPort.available() >= 18) {  // If data is available
    val=decodeRXAPIpacket(val); // try to decode the received API frame from XBEE
    dataProcessing(val);
  }

  //Contagem de Carros
  drawContCars(360, 150, contNorth);
  drawContCars(610, 720, contSouth);
  drawContCars(800, 325, contEast);
  drawContCars(210, 580, contWest);

  //Velocidade Média
  drawSpeed(130, 580, speedWest);

  //Botão Pressionado
  drawBtnPressed(50, 50, btnPressed);

  //RSSI
  drawRSSIdisplay(200, 50, rssi);
}

// Called when a key button is pressed
void keyPressed() {
  if (key == 'R' || key == 'r') {
    formatTXAPIpacket((byte) 'R'); 
  }
}

// Traduz o código em ações
// Em caso de dúvida, chamo a atenção para a linha 3, 4, e 6
int dataProcessing(int val) {
  formatTXAPIpacket((byte) 'R'); // Turn on LED
  println("Data Processing - Val");
  println(val);
  
  int cod = val % 100;
  
  println("Data Processing - Cod");
  println(cod);

  switch (cod) {
    // Semáforo North
  case 10:
    println("Semáforo Norte -->");
    val = val - 10;
    val = val / 100;

    switch(val) {
    case 1:
      northStoplight.turnRed();
      break;
    case 2:
      northStoplight.turnYellow();
      break;
    case 3:
      northStoplight.turnGreen();
      break;
    }
    break;

    // Semáforo South
  case 20:
    println("Semáforo Sul -->");
    val = val - 20;
    val = val / 100;

    switch(val) {
    case 1:
      southStoplight.turnRed();
      break;
    case 2:
      southStoplight.turnYellow();
      break;
    case 3:
      southStoplight.turnGreen();
      break;
    }
    break;

    //Semáforo Este
  case 30:
    println("Semáforo Este -->");
    val = val - 30;
    val = val / 100;

    switch(val) {
    case 1:
      eastStoplight.turnRed();
      break;
    case 2:
      eastStoplight.turnYellow();
      break;
    case 3:
      eastStoplight.turnGreen();
      break;
    }
    break;

    //Semáforo Oeste
  case 40:
    println("Semáforo Oeste -->");
    val = val - 40;
    val = val / 100;

    switch(val) {
    case 1:
      westStoplight.turnRed();
      break;
    case 2:
      westStoplight.turnYellow();
      break;
    case 3:
      westStoplight.turnGreen();
      break;
    }
    break;

    // Semáforo Peões
  case 50:
    println("Semáforo Peões -->");
    val = val - 50;
    val = val / 100;

    switch(val) {
    case 1:
      stoplightPawnsNorthA.turnOn();
      stoplightPawnsNorthB.turnOn();
      break;
    case 2:
      stoplightPawnsNorthA.turnOff();
      stoplightPawnsNorthB.turnOff();
      break;
    }

    // Contador de Carros nas Estradas
  case 60:
    println("Passagem de Carros -->");
    val = val - 60;
    val = val / 100;

    switch(val) {
    case 1:
      contNorth = contNorth + 1;
      break;
    case 2:
      contSouth = contSouth + 1;
      break;
    case 3:
      contEast = contEast + 1;
      break;
    case 4:
      contWest = contWest + 1;
    }
    break;

    // Acionamento do Botão de Peão
  case 70:
    println("Botão de Peões -->");
    val = val - 70;
    val = val / 100;

    switch(val) {
    case 1:
      btnPressed = true;
      break;
    case 2:
      btnPressed = false;
      break;
    }
    break;

    // Velocidade Média
  case 80:
    println("Velocidade Média -->");
    val = val - 80;
    val = val / 100;
    speedWest = val;
    println(speedWest);
    break;
  }
  return val;
}


//Semáforos Norte
public class stoplightNorth {
  int xpos;
  int ypos;

  stoplightNorth(int x, int y) {
    xpos = x;
    ypos = y;
  }

  void display() {
    // Retangulo
    fill(0);
    rect(xpos, ypos, 35, 80);

    //Circulo Vermelho
    fill(255, 0, 0);//red circle for red light
    ellipse( xpos+18, ypos+65, 20, 20);//fade to represent it's off
    fill(50, 200);//faded grey
    ellipse( xpos+18, ypos+65, 20, 20);

    // Circulo Amarelo
    fill(255, 255, 0);//Yellow circle for Yellow light
    ellipse( xpos+18, ypos+40, 20, 20);
    fill(50, 200);//Faded grey to represent it's off
    ellipse( xpos+18, ypos+40, 20, 20);

    //Circulo Verde
    fill(0, 255, 0); //Green Circle
    ellipse( xpos+18, ypos+15, 20, 20);
    fill(50, 200);//Faded grey to represent it's off
    ellipse( xpos+18, ypos+15, 20, 20);
  }

  void turnRed() {
    display();
    fill(255, 0, 0);//red circle for red light
    ellipse( xpos+18, ypos+65, 20, 20);//fade to represent it's off
  }

  void turnYellow() {
    display();
    fill(255, 255, 0);//Yellow circle for Yellow light
    ellipse( xpos+18, ypos+40, 20, 20);
  }

  void turnGreen() {
    display();
    fill(0, 255, 0); //Green Circle
    ellipse( xpos+18, ypos+15, 20, 20);
  }
}

//Semáforos South
public class stoplightSouth {
  int xpos;
  int ypos;

  stoplightSouth(int x, int y) {
    xpos = x;
    ypos = y;
  }

  void display() {
    // Retangulo
    fill(0);
    rect(xpos, ypos, 35, 80);

    //Circulo Vermelho
    fill(255, 0, 0);//red circle for red light
    ellipse( xpos+18, ypos+15, 20, 20);//fade to represent it's off
    fill(50, 200);//faded grey
    ellipse( xpos+18, ypos+15, 20, 20);

    // Circulo Amarelo
    fill(255, 255, 0);//Orange circle for orange light
    ellipse( xpos+18, ypos+40, 20, 20);
    fill(50, 200);//Faded grey to represent it's off
    ellipse( xpos+18, ypos+40, 20, 20);

    //Circulo Verde
    fill(0, 255, 0);//Green Circle
    ellipse( xpos+18, ypos+65, 20, 20);
    fill(50, 200);//Faded grey to represent it's off
    ellipse( xpos+18, ypos+65, 20, 20);
  }

  void turnRed() {
    display();
    fill(255, 0, 0);//red circle for red light
    ellipse( xpos+18, ypos+15, 20, 20);
  }

  void turnYellow() {
    display();
    fill(255, 255, 0);//Yellow circle for Yellow light
    ellipse( xpos+18, ypos+40, 20, 20);
  }

  void turnGreen() {
    display();
    fill(0, 255, 0);//Green Circle
    ellipse( xpos+18, ypos+65, 20, 20);
  }
}

//Semáforos Este 
public class stoplightEast {
  int xpos;
  int ypos;

  stoplightEast(int x, int y) {
    xpos = x;
    ypos = y;
  }

  void display() {
    // Retangulo
    fill(0);
    rect(xpos, ypos, 80, 35);

    //Circulo Vermelho
    fill(255, 0, 0);//red circle for red light
    ellipse( xpos+18, ypos+16, 20, 20);//fade to represent it's off
    fill(50, 200);//faded grey
    ellipse( xpos+18, ypos+16, 20, 20);

    // Circulo Amarelo
    fill(255, 255, 0);//Yellow circle for Yellow light
    ellipse( xpos+40, ypos+16, 20, 20);
    fill(50, 200);//Faded grey to represent it's off
    ellipse( xpos+40, ypos+16, 20, 20);

    //Circulo Verde
    fill(0, 255, 0);
    ellipse( xpos+65, ypos+16, 20, 20);
    fill(50, 200);
    ellipse( xpos+65, ypos+16, 20, 20);
  }

  void turnRed() {
    display();
    fill(255, 0, 0);//red circle for red light
    ellipse( xpos+18, ypos+15, 20, 20);
  }

  void turnYellow() {
    display();
    fill(255, 255, 0);//Yellow circle for Yellow light
    ellipse( xpos+40, ypos+16, 20, 20);
  }

  void turnGreen() {
    display();
    fill(0, 255, 0);
    ellipse( xpos+65, ypos+16, 20, 20);
  }
}

public class stoplightWest {
  int xpos;
  int ypos;

  stoplightWest(int x, int y) {
    xpos = x;
    ypos = y;
  }

  void display() {
    // Retangulo
    fill(0);
    rect(xpos, ypos, 80, 35);

    //Circulo Vermelho
    fill(255, 0, 0);//red circle for red light
    ellipse( xpos+65, ypos+16, 20, 20);//fade to represent it's off
    fill(50, 200);//faded grey
    ellipse( xpos+65, ypos+16, 20, 20);

    // Circulo Amarelo
    fill(255, 255, 0);//Yellow circle for Yellow light
    ellipse( xpos+40, ypos+16, 20, 20);
    fill(50, 200);//Faded grey to represent it's off
    ellipse( xpos+40, ypos+16, 20, 20);

    //Circulo Verde
    fill(0, 255, 0);
    ellipse( xpos+18, ypos+16, 20, 20);
    fill(50, 200);
    ellipse( xpos+18, ypos+16, 20, 20);
  }

  void turnRed() {
    display();
    fill(255, 0, 0);//red circle for red light
    ellipse( xpos+65, ypos+16, 20, 20);//fade to represent it's off
  }

  void turnYellow() {
    display();
    fill(255, 255, 0);//Yellow circle for Yellow light
    ellipse( xpos+40, ypos+16, 20, 20);
  }

  void turnGreen() {
    display();
    fill(0, 255, 0);
    ellipse( xpos+18, ypos+16, 20, 20);
  }
}

public class stoplightPawns {
  int xpos;
  int ypos;

  stoplightPawns(int x, int y) {
    xpos = x;
    ypos = y;
  }

  void display() {
    // Retangulo
    fill(0);
    rect(xpos, ypos, 35, 35);

    // Circulo Amarelo
    fill(255, 255, 0);//Yellow circle for Yellow light
    ellipse( xpos+18, ypos+16, 25, 25);
    fill(50, 200);//Faded grey to represent it's off
    ellipse( xpos+16, ypos+16, 25, 25);
  }

  void turnOn() {
    fill(255, 255, 0);//Yellow circle for Yellow light
    ellipse( xpos+18, ypos+16, 25, 25);
  }
  void turnOff() {
    display();
  }
}

// Interface de Contagem de Carros
void drawContCars(int xpos, int ypos, int cont) {
  fill(#808080);
  rect(xpos, ypos, 45, 30);
  fill(#000000);  // write text in white
  text("Carros: ", xpos, ypos-5);
  fill(#000000);  // write text in white
  text(cont, xpos + 5, ypos+25);
}

// Interface de Velocidade Média - M/s
void drawSpeed(int xpos, int ypos, float value) {
  fill(#808080);
  rect(xpos, ypos, 50, 30);
  fill(#000000);  // write text in white
  text("Velocidade: ", xpos, ypos-5);
  fill(#000000);  // write text in white
  text(value, xpos + 5, ypos+25);
}

// Interface de Botão Pressionado
void drawBtnPressed(int xpos, int ypos, boolean btnPressed) {
  String textDisplay;

  if (btnPressed) {
    textDisplay = "Sim";
  } else {
    textDisplay = "Não";
  }

  fill(#808080);
  rect(xpos, ypos, 50, 30);
  fill(#000000);  // write text in white
  text("Botão Pressionado: ", xpos, ypos-5);
  fill(#000000);  // write text in white
  text(textDisplay, xpos + 5, ypos+25);
}

// Interface de RSSI
void drawRSSIdisplay(int xpos, int ypos, int rssi) {
  // show last RSSI value inside a rectangle:
  fill(#808080);
  rect(xpos, ypos, 60, 30);
  fill(#000000); 
  text("RSSI of Remote XBEE: ", xpos, ypos-5);
  fill(#000000);  // write text in white
  text("-" + rssi + " dBm", xpos+5, ypos+25); // Value stored in rssi is |rssi| -> real value is negative
}


// XBEE
void formatTXAPIpacket(byte value) {
  // Transmit key pressed using XBEE API frame

  int sum = 0; // Accumulate the checksum  

  // API frame Start Delimiter
  myPort.write(frameStartByte);

  // Length - High and low parts of the frame length (Number of bytes between the length and the checksum)
  myPort.write((byte) 0x00);
  myPort.write((byte) 0x0F);

  // Frame Type - Indicate this frame contains a Transmit Request
  myPort.write(frameTypeTXrequest); 
  sum += frameTypeTXrequest;

  // Frame ID - set to zero for no reply
  myPort.write((byte) 0x00); 
  sum += 0x00;

  // 64-bit Destination Address - The following 8 bytes indicate the 64 bit address of the recipient (high and low parts).
  // Use 0xFFFF to broadcast to all nodes.
  myPort.write((byte) ((destAddressHigh >> 24) & 0xFF));
  myPort.write((byte) ((destAddressHigh >> 16) & 0xFF));
  myPort.write((byte) ((destAddressHigh >> 8) & 0xFF));
  myPort.write((byte) ((destAddressHigh) & 0xFF));
  sum += (byte) ((destAddressHigh >> 24) & 0xFF);
  sum += (byte) ((destAddressHigh >> 16) & 0xFF);
  sum += (byte) ((destAddressHigh >> 8) & 0xFF);
  sum += (byte) (destAddressHigh & 0xFF);
  myPort.write((byte) ((destAddressLow >> 24) & 0xFF));
  myPort.write((byte) ((destAddressLow >> 16) & 0xFF));
  myPort.write((byte) ((destAddressLow >> 8) & 0xFF));
  myPort.write((byte) ((destAddressLow & 0xFF)));
  sum += ((byte) (destAddressLow >> 24) & 0xFF);
  sum += ((byte) (destAddressLow >> 16) & 0xFF);
  sum += ((byte) (destAddressLow >> 8) & 0xFF);
  sum += ((byte) (destAddressLow & 0xFF));

  // 16-bit Destination Network Address - The following 2 bytes indicate the 16-bit address of the recipient.
  // Use 0xFFFE if the address is unknown.
  myPort.write((byte)0xFF);
  myPort.write((byte)0xFE);
  sum += 0xFF+0xFE;

  // Broadcast Radius - when set to 0, the broadcast radius will be set to the maximum hops value
  myPort.write((byte)0x00);
  sum += 0x00;

  // Options
  // 0x01 - Disable retries and route repair
  // 0x20 - Enable APS encryption (if EE=1)
  // 0x40 - Use the extended transmission timeout
  myPort.write((byte)0x00);
  sum += 0x00;

  // RF Data
  myPort.write(value);
  sum += value;

  // Checksum = 0xFF - the 8 bit sum of bytes from offset 3 (Frame Type) to this byte.
  myPort.write( (byte) (0xFF - ( sum & 0xFF)));
}

int decodeRXAPIpacket(int val) {
  // Function for decoding the received API frame from XBEE
  int rxvalue=0;

  while (myPort.read () != frameStartByte) {
    if (myPort.available()==0)
      return val; // No API frame present.
  }

  // Skip over the bytes in the API frame we don't care about (length, frame type, addresses, receive options)
  for (int i = 0; i < 14; i++) {
    myPort.read();
  }
  // The next two bytes represent the ADC measurement sent from the remote XBEE
  rxvalue = myPort.read() * 256; // add the most significant byte
  rxvalue += myPort.read(); // read the least significant byte

  // The next byte represents the rssi measurement sent from the remote XBEE
  rssi = myPort.read();


  myPort.read(); // Read  the last byte (Checksum) but don't store it

  return rxvalue;
}
