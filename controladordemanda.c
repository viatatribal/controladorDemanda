#include "WiFi.h"
#include "ModbusIP_ESP8266.h"
#include <DS1307.h> // para o módulo RTC DS3231

// variaveis para definir hora e data no módulo RTC DS3231
uint8_t set_Sec = 0;        		// Definir segundos
uint8_t set_Minute = 42;    	// Definir minutos
uint8_t set_Hour = 17;       	// Definir hora
uint8_t set_Day = 8;       		// Definir dia
uint8_t set_Month = 1;     	// Definir mês
uint16_t set_Year = 2025; 	// Definir ano

// variaveis para pegar hora e data do módulo RTC DS3231
uint8_t sec, minute, hour, day, month;
uint16_t year;
DS1307 rtc;

// variaveis de controle do relé
uint8_t desarmeHora = 17;		    // hora para desarmar
unsigned long intervaloMaximo = 8000; // 8 segundos

// Configurações da rede WiFi
const char* ssid = "TEE_420A";  	// Nome da rede WiFi
const char* password = "@uff420a";  // Senha da rede WiFi

// Configuração do IP estático
IPAddress local_IP(192, 168, 1, 59);  	// IP fixo do ESP32
IPAddress gateway(192, 168, 1, 1);    	// Gateway padrão (ajuste conforme sua rede)
IPAddress subnet(255, 255, 255, 0);   	// Máscara de sub-rede

// Configuração do Modbus TCP/IP
ModbusIP mb;
const int REG_INTERVAL = 101;     	// Registro Modbus para intervalo entre pulsos
const int REG_RELE1 = 104;        		// Registro Modbus para relé 1

// Configuração do I/O
#define PULSE_PIN 18  			// Pino que está recebendo o pulso
#define RELE1 23   				// Relé

// configuração do tempo entre loops
unsigned long lastMillis = 0; 			  // Armazena o tempo da última execução
unsigned long intervalMillis = 1000; 		 // Intervalo de 100ms (pode ser ajustado)

// Variáveis de controle de pulsos
volatile unsigned long lastPulseTime = 0;  	   // Tempo do último pulso
volatile unsigned long pulseInterval = 0;  	   // Intervalo entre os pulsos

// Função de interrupção para contagem de pulsos
void IRAM_ATTR pulseISR() {
  unsigned long currentTime = millis();
  pulseInterval = currentTime - lastPulseTime;  // Calcula o intervalo
  lastPulseTime = currentTime;              		 // Atualiza o tempo do último pulso
}

// Conecta ao WiFi com IP fixo
void connectToWiFi() {
  Serial.println("Conectando ao WiFi...");
  if (!WiFi.config(local_IP, gateway, subnet)) {
	Serial.println("Falha ao configurar IP estático!");
  }
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
  }

  Serial.println("\nConectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  // Conexão WiFi
  connectToWiFi();
  mb.server();

  // Configuração do pino de entrada e interrupção
  pinMode(PULSE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), pulseISR, FALLING);

  // Inicializa o servidor Modbus TCP
  mb.connect(local_IP);  			// Configura IP fixo e porta Modbus
  mb.addHreg(REG_INTERVAL, 0);  	// Registro para intervalo entre pulsos
  mb.addCoil(REG_RELE1, false);


  Serial.println("Servidor Modbus iniciado.");

  pinMode(RELE1, OUTPUT);

  digitalWrite(RELE1, LOW);

  rtc.begin();
  // definir hora e data no módulo RTC DS3231
  // essa linha de código deve ser usada apenas na primeira vez que o programa
  // é enviado ao ESP32, após isso, deve ser comentada para não
  // ser utilizada mais
  //rtc.set(set_Sec, set_Minute, set_Hour, set_Day, set_Month, set_Year);
  rtc.start();            	/start RTC/

  delay(10000); // para nao desarmar o relé rapido demais
}

void loop() {
   unsigned long currentMillis = millis();
  // Verifica se já passou o intervalo
  if (currentMillis - lastMillis >= intervalMillis) {
  lastMillis = currentMillis; // Atualiza o tempo da última execução

  unsigned long currentPulseCount;
  unsigned long interval;

  // Desabilita interrupções temporariamente para leitura estável
  noInterrupts();
  interval = pulseInterval;
  interrupts();

  // Atualiza os registros Modbus
  mb.Hreg(REG_INTERVAL, (uint16_t)interval);

  Serial.print("Intervalo (s): ");
  Serial.println(interval);

   // pega hora e data atual
  rtc.get(&sec, &minute, &hour, &day, &month, &year);

  Serial.print("\nHora: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.print(sec);

  Serial.print("\nData: ");
  Serial.print(day);
  Serial.print(".");
  Serial.print(month);
  Serial.print(".");
  Serial.print(year);
  Serial.println("");

  if (interval > 0 && interval <= intervaloMaximo && hour >= desarmeHora)
  {
      digitalWrite(RELE1, HIGH);
  }

  // Gerencia o servidor Modbus TCP
  mb.task();
  }
}
