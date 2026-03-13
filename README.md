📝 Sobre o Projeto
Sistema embarcado que monitora frequência cardíaca, oxigenação (SpO₂) e temperatura corporal usando um ESP32 e sensores médicos. Os dados são coletados e enviados via ESP-NOW para outro ESP32.

🔧 Componentes Necessários
1x ESP32

1x Sensor MAX30102 (coração e oxigênio)

1x Sensor MAX30205 (temperatura)

Jumpers e protoboard

🔌 Ligações (I2C)
Sensor	Pino	ESP32
MAX30102	SDA	GPIO 21
MAX30102	SCL	GPIO 22
MAX30205	SDA	GPIO 21
MAX30205	SCL	GPIO 22
Ambos	VIN	3.3V
Ambos	GND	GND
📥 Instalação
Instale a Arduino IDE

Adicione suporte ao ESP32 (caso não tenha)

Instale as bibliotecas:

MAX30102_PulseOximeter

ClosedCube_MAX30205

Baixe este código e abra no Arduino IDE

Altere o MAC de destino (linha receiverMac[])

Carregue no ESP32

📊 Como Funciona
A cada 1 segundo: lê os sensores

A cada 2 segundos: envia os dados via ESP-NOW

Os dados enviados são:

Código do dispositivo

Batimentos cardíacos (bpm)

Saturação de oxigênio (%)

Temperatura (°C)

▶️ Como Usar
Conecte o ESP32 ao computador

Abra o Monitor Serial (115200 baud)

Coloque o dedo no sensor MAX30102

Aguarde as leituras aparecerem

📦 Estrutura dos Dados Enviados
cpp
{
  code: 123456,  // identificador
  hr: 78.5,      // batimentos
  spo: 97.2,     // oxigenação
  temp: 36.1     // temperatura
}
🎯 Aplicações
Monitoramento remoto de pacientes

Projetos de telemedicina

Estudos de biossinais

Protótipos de dispositivos wearables
