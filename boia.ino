#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "rede"; // Nome da sua rede Wi-Fi
const char* password = "*****"; // Senha da sua rede Wi-Fi

const int trigPin = 12;
const int echoPin = 14;
const int profundiade = 60;
#define SOUND_VELOCITY 0.034

long duration;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Conecta-se à rede Wi-Fi
  reconectarWiFi();

  Serial.println("Conectado à rede Wi-Fi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  server.on("/data", HTTP_GET, []() {
    int distance = media(); // Obtém a média das distâncias
    String json = "{\"porcentagem\": " + String(distance) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  server.handleClient();

  // Verifica se a conexão WiFi foi perdida e tenta reconectar
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexão perdida. Tentando reconectar...");
    reconectarWiFi();
  }
}

void reconectarWiFi() {
  WiFi.begin(ssid, password);
  int tentativas = 0;

  while (WiFi.status() != WL_CONNECTED && tentativas < 10) {
    delay(1000);
    tentativas++;
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nReconexão bem-sucedida!");
  } else {
    Serial.println("\nFalha na reconexão. Verifique a conexão e reinicie.");
  }
}

int media() {
  int Vetor[20];
  for (int i = 0; i < 20; i++) {
    Vetor[i] = hcsr04();
    delay(50);
  }

  // Ordena o vetor de distâncias em ordem crescente
  for (int i = 0; i < 19; i++) {
    for (int j = 0; j < 19 - i; j++) {
      if (Vetor[j] > Vetor[j + 1]) {
        int temp = Vetor[j];
        Vetor[j] = Vetor[j + 1];
        Vetor[j + 1] = temp;
      }
    }
  }

  // Calcula a média das distâncias restantes (após remover as 3 menores e 3 maiores)
  int soma = 0;
  for (int i = 3; i < 17; i++) {
    soma += Vetor[i];
  }
  int distanciaMedia = soma / 14;

  // Verifica a porcentagem de água na caixa d'água
  if (distanciaMedia >= profundiade) {
    return 0; // Se a distância for maior ou igual à profundidade, caixa d'água está em 0%
  } else if (distanciaMedia < 10) {
    return 100; // Se a distância for menor que 10 cm, caixa d'água está em 100%
  } else {
    // Calcula a porcentagem com base na distância média
    int porcentagem = map(distanciaMedia, 10, profundiade, 100, 0);
    return constrain(porcentagem, 0, 100); // Garante que a porcentagem está entre 0% e 100%
  }
}


int hcsr04() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  return duration * SOUND_VELOCITY / 2;
}
