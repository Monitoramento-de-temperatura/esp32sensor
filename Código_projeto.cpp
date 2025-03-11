/*
Programa: Sistema de Monitoramento de Temperatura para ambiente climatizado por ar condicionado do tipo Chiller
Autores: Adriel Vidal e Alessandra Félix
Data: 27/02/2025
Descrição: Este programa utiliza o microcontrolador ESP32, realiza a leitura de dados do sensor de temperatura DHT11, 
mostra na tela Oled instalada e compara com a faixa de temperatura estipulada previamente, 
se estiver fora da faixa estipulada este programa envia um e-mail de alerta ao usuário designado
e reenvia um e-mail a cada hora caso a temperatura continue fora da faixa. 
Independente disto, ele envia constantemente os dados de temperatura para o site ThingSpeak, 
onde é criado um gráfico para que o usuário possa acompanhar as oscilações de temperatura em tempo real.
*/

#include <WiFi.h> // Biblioteca para conectar o ESP32 a redes Wi-Fi
#include <ESP_Mail_Client.h> // Biblioteca para enviar e-mails pelo ESP32
#include <DHT.h> // Biblioteca para leitura de sensores DHT
#include <Wire.h> // Biblioteca para comunicação do tipo I2C
#include <Adafruit_GFX.h> // Biblioteca gráfica da Adafruit para manipulação de elementos gráficos
#include <Adafruit_SSD1306.h> // Biblioteca específica para displays OLED SSD1306 via comunicação I2C
#include <HTTPClient.h> // Biblioteca para realizar requisições HTTP, como envio de dados para o ThingSpeak

// Configuração do Wi-Fi
#define WIFI_SSID "INSIRA_O_SSID_DO_WIFI"
#define WIFI_PASSWORD "INSIRA_A_SENHA_DO_WIFI"

// Configuração do e-mail usando SMTP, para enviar os e-mails de alerta para o usuário
#define SMTP_SERVER "INSIRA_O_SMTP_SERVER_DO_E-MAIL"
#define SMTP_PORT "INSIRA_A_PORTA_DO_SMTP"
#define AUTHOR_EMAIL "INSERIR_E-MAIL_DO_AUTOR_COM_SENHA_DE_APP"
#define AUTHOR_PASSWORD "INSIRA_A_SENHA_DE_APP_DO_EMAIL_DO_AUTOR"
#define RECIPIENT_EMAIL "INSIRA_O_E-MAIL_QUE_RECEBERA_OS_ALERTAS"

// Configuração do site ThingSpeak que faz o gráfico da temperatura em tempro real
#define THINGSPEAK_API_KEY "INSIRA_API_KEY_DO_THINGSPEAK"
#define THINGSPEAK_URL "http://api.thingspeak.com/update"

// Configuração das variáveis de controle que envia de 15 em 15 segundos os dados para o site ThingSpeak
unsigned long tempoUltimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 15000; // 15 segundos

// Configuração do Sensor de Temperatura DHT11
#define DHTPIN 4 // define o pino 4 do ESP32 para receber os dados do sensor
#define DHTTYPE DHT11 // define o modelo do sensor, usamos DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configuração do Display OLED
#define SCREEN_WIDTH 128 //define a largura da tela
#define SCREEN_HEIGHT 64 // define a altura da tela
#define OLED_RESET -1 // informa que a tela não possui pino de RESET
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configuração dos objetos de SMTP
SMTPSession smtp; // Configura a conexão com o servidor de e-mail
ESP_Mail_Session session; // Configura as informações da conexão com o servidor
SMTP_Message message; // Prepara a mensagem de e-mail que será enviada

// Variável para controle do e-mail de alerta de 1 em 1 hora
struct Alerta {
    bool enviado;
    unsigned long ultimaNotificacao;
} alerta = {false, 0};
const unsigned long INTERVALO_EMAIL = 3600000; // intervalo de 1 hora
unsigned long tempoUltimaVerificacao = 0; 

// Função para fazer a conexão do Wi-fi
void conectarWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Nome e senha do Wi-fi
    Serial.print("Conectando ao Wi-Fi..."); // Mostra esta informação no monitor serial
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Conectando ao Wi-Fi");
    display.display();

    int tentativas = 0; // Tenta conectar 20 vezes no Wi-fi
    while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
        delay(500);
        Serial.print(".");
        tentativas++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWi-Fi Conectado!"); // Mostra esta informação no monitor serial
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Wi-Fi Conectado");
        display.display();
    } else {
        Serial.println("\nFalha na conexão Wi-Fi. Reiniciando..."); // Mostra esta informação no monitor serial
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Falha no Wi-Fi");
        display.println("Reiniciando");
        display.display();
        ESP.restart(); // Se ele não conectar no Wi-fi ele vai reiniciar o EPS32
    }
}

// Configuração do ESP32
void setup() {
    Serial.begin(115200);
    dht.begin();
    // Inicialização do display OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Falha ao inicializar OLED!");
        while (true);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Iniciando");
    display.display();

    conectarWiFi();
    
    // Configuração da sessão SMTP
    session.server.host_name = SMTP_SERVER; // Nome do servidor SMTP
    session.server.port = SMTP_PORT; // Porta do servidor SMTP
    session.login.email = AUTHOR_EMAIL; // e-mail criado para envio de alertas
    session.login.password = AUTHOR_PASSWORD; // senha do e-mail criado para envio de alertas
    session.login.user_domain = ""; // domínio do usuário
    session.time.ntp_server = "pool.ntp.org"; // Define o servidor NTP para sincronização de tempo
    session.time.gmt_offset = -3; // Fuso horário (GMT-3)
    
    smtp.debug(1); // Ativa o modo de depuração 
    smtp.callback(smtpCallback); // Define uma função de callback
}

// Inicializando o loop
void loop() {
    unsigned long tempoAtual = millis(); // Esta função retorna o número de milissegundos que se passaram desde que o ESP32 foi ligado ou reiniciado.

    // Conectar no Wi-fi
    if (WiFi.status() != WL_CONNECTED) {
        conectarWiFi();
    }


    // Faz a leitura da Temperatura
    float temperatura = dht.readTemperature();
    if (isnan(temperatura)) {
        Serial.println("Erro ao ler DHT11!"); // Exibe no monitor serial se tiver um erro de leitura do sensor DHT11
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Erro ao ler DHT11");
        display.display();
        return;
    }

    Serial.print("Temperatura: ");
    Serial.println(temperatura);

    // Atualiza o display OLED com a temperatura
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Temperatura:");
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.print(temperatura, 2);  // Exibe temperatura com duas casas decimais
    display.print(" ");
    display.print((char)247);  // Símbolo de grau (°) do ASCII
    display.print("C");
    display.display();

    // Envia os dados para o site ThingSpeak no intervalo de 15 em 15 segundos
    if (tempoAtual - tempoUltimoEnvio >= INTERVALO_ENVIO) {
        enviarParaThingSpeak(temperatura);
        tempoUltimoEnvio = tempoAtual;
    }

    // Verifica a cada 30 segundos a temperatura para fazer o envio do e-mail de alerta
    if (tempoAtual - tempoUltimaVerificacao >= 30000) { 
        float temperatura = dht.readTemperature();
        if (!isnan(temperatura) && (temperatura < 20.0 || temperatura > 23.0)) { // Verifica se a leitura da temperatura é válida e se está fora do intervalo estipulado          
        if (!alerta.enviado || (tempoAtual - alerta.ultimaNotificacao >= INTERVALO_EMAIL)) {
                enviarEmail(temperatura);
                alerta.enviado = true;
                alerta.ultimaNotificacao = tempoAtual;
            }
        } else {
            alerta.enviado = false;
        }
        tempoUltimaVerificacao = tempoAtual;
    }

    delay(500);
}
// Função que envia os dados de temperatura para o site ThingSpeak
void enviarParaThingSpeak(float temp) {
    if (WiFi.status() == WL_CONNECTED) {
        String url = String(THINGSPEAK_URL) + "?api_key=" + THINGSPEAK_API_KEY + "&field1=" + String(temp); //cria uma URL para enviar os dados ao Thingspeak
        HTTPClient http;
        http.begin(url); // incializa a conexão HTTP com a URL
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            Serial.println("Enviado para ThingSpeak!");
        } else {
            Serial.println("Erro ao enviar para ThingSpeak!");
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0, 0);
            display.println("Erro ao enviar para ThingSpeak");
            display.println("Reiniciando");
            display.display();
            ESP.restart();
        }
        http.end();
    }
}

// Função para enviar e-mail de Alerta
void enviarEmail(float temp) {
    message.sender.name = "Monitoramento de Temperatura";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = "⚠️ Alerta de Temperatura!";
    message.addRecipient("Destinatario", RECIPIENT_EMAIL);
    
    String emailBody = "Informamos que a temperatura está fora do limite aceitável.\n\n";
    emailBody += "Temperatura atual: " + String(temp, 2) + " °C\n\nAcesse o gráfico em tempo real: https://thingspeak.mathworks.com/channels/2859751";
    message.text.content = emailBody.c_str();
 

    // Se a conexão falhar imprime mensagem de erro no serial
    if (!smtp.connect(&session)) {
        Serial.println("Falha ao conectar ao servidor SMTP!");
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Falha ao conectar ao SMTP");
        display.display();
        return;
    }
    // Tenta enviar o e-mail, se não conseguir exibe mensagem de erro
    if (!MailClient.sendMail(&smtp, &message)) {
        Serial.println("Erro ao enviar e-mail: " + smtp.errorReason());
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Erro ao enviar e-mail");
        display.display();
    } else {
        Serial.println("E-mail enviado com sucesso!");
        display.setTextSize(1);
        display.setCursor(0, 40);
        display.println("E-mail enviado!");
        display.display();
        delay(5000);
    }
    smtp.sendingResult.clear(); // Limpa o resultado do envio do e-mail de alerta
}

void smtpCallback(SMTP_Status status) {
    Serial.println(status.info());
}

