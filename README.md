# **Sistema de Monitoramento de Temperatura para ambiente climatizado por ar condicionado do tipo Chiller**

## **Descrição**

Este projeto é um Sistema de Monitaramento de Temperatura para um ambiente climatizado por ar condicionado do tipo Chiller, 
desenvolvido em C++ que utiliza o microcontrolador ESP32, realiza a leitura de dados do sensor de temperatura DHT11, 
mostra na tela Oled instalada e compara com a faixa de temperatura estipulada previamente, 
se estiver fora da faixa estipulada este programa envia um e-mail de alerta ao usuário designado e 
reenvia um e-mail a cada hora caso a temperatura continue fora da faixa. 
Independente disto, ele envia constantemente os dados de temperatura para o site ThingSpeak, 
onde é criado um gráfico para que o usuário possa acompanhar as oscilações de temperatura em tempo real.

## **Funcionalidades**
- Leitura de temperatura usando o sensor DHT11  
- Exibição da temperatura na tela OLED  
- Comparação com a faixa de temperatura estipulada  
- Envio de e-mail ao usuário se a temperatura sair da faixa aceitável  
- Reenvio de e-mail a cada 1 hora se a temperatura continuar fora do limite  
- Upload de dados para o ThingSpeak para monitoramento remoto  

## **Como executar**
### **Pré-requisitos**
Antes de executar o projeto, certifique-se de ter instalado:  
- [Arduino IDE](https://www.arduino.cc/en/software)  
- Biblioteca `WiFi.h` para conectar o ESP32 a redes Wi-Fi
- Biblioteca `ESP32_MailClient` para envio de e-mails  
- Biblioteca `DHT` para leitura do sensor DHT
- Biblioetaca `Wire.h` para comunicação do tipo I2C
- Biblioteca `Adafruit_GFX` e `Adafruit_SSD1306` para controle da tela OLED  
- Biblioetaca `HTTPClient.h` para realizar requisições HTTP
- Conta de e-mail com senha para aplicativo
- Conta e API configuradas no [ThingSpeak](https://thingspeak.com/)  

### **Passos para execução**
1. **Clone o repositório**  
   ```bash
   git clone https://github.com/Monitoramento-de-temperatura/esp32sensor.git
2. Abra o projeto na Arduino IDE
3. Instale as bibliotecas necessárias
4. Configure as informações de Wi-Fi e e-mail no código para envio e recebimento dos e-mails de alerta
5. Configure a formatação da Partição do ESP32 para Huge APP (3MB No OTA/1MB SPIFFS)
5. Compile e carregue o código no ESP32
6. Abra o Monitor Serial para verificar os logs
7. Acompanhe a temperatura no display OLED
8. Acompanhe os e-mails de alerta enviados
9. Acompanhe os dados no ThingSpeak


## **Tecnologias utilizadas**
- Linguagem: C++
- Microcontrolador: ESP32
- Sensor: DHT11
- Display: OLED SSD1306
- Plataforma IoT: ThingSpeak
- Bibliotecas:
  - DHT.h - Leitura do sensor de temperatura
  - ESP32_MailClient.h - Envio de e-mails
  - WiFi.h - Conexão com a rede Wi-Fi
  - HTTPClient.h  - Comunicação com ThingSpeak
  - Adafruit_GFX.h e Adafruit_SSD1306.h - Controle da tela OLED


## **Autores**
- Nomes: Adriel Vidal e Alessandra Félix 
- Email: monitoramentotemperatura7@gmail.com
- GitHub: https://github.com/Monitoramento-de-temperatura

## **Licença**
Este projeto é licenciado sob a Licença MIT. Para mais informações, consulte o arquivo LICENSE.
