Hardware
Qtd	Componente	Função
1	ESP32 WROOM-32, 38 pinos	Controle
1	Ponte H TB6612FNG	Acionamento dos motores
2	Micro motor N20 12 V (300 a 600 rpm) com rodas	Tração
2	Sensor TCRT5000	Detecção da borda branca
2	Sensor ultrassônico HC-SR04	Detecção do adversário
3	Bateria 18650 (3,7 V nominal) com suporte	Alimentação (11,1 V nominal, 12,6 V carregada)
1	Regulador DC-DC LM2596 (HW-411)	12 V para 5 V
1	Chave liga/desliga	Alimentação geral
1	Protoboard de 400 furos e jumpers Dupont	Distribuição de energia e sinais
2 + 2	Resistores de 1 kOhm (2 unidades) e de 2 kOhm (2 unidades)	Divisores de tensão do Echo (por sensor: 1x 1 kOhm e 1x 2 kOhm)
Alimentação
Bateria 3x18650 -> chave -+-> TB6612FNG (VM) -> motores (12 V)
                          |
                          +-> LM2596 (ajustado em 5,0 V) -> ESP32 (pino 5V)
                                                          -> TB6612FNG (VCC, STBY)
                                                          -> HC-SR04 (Vcc)

ESP32 (pino 3V3) -> TCRT5000 (VCC)
GND comum: bateria, LM2596, ponte H, ESP32 e sensores
Pinagem

Todos os sinais saem do lado esquerdo do ESP32 (com a antena para cima e o USB-C para baixo).

Ponte H TB6612FNG
Sinal	GPIO do ESP32
PWMA (motor esquerdo)	25
AIN1	26
AIN2	27
PWMB (motor direito)	33
BIN1	32
BIN2	13
STBY	fixo em 5 V
Sensores
Sinal	GPIO do ESP32
TCRT5000 frente (D0)	34
TCRT5000 trás (D0)	35
HC-SR04 Trig (compartilhado pelos dois)	14
HC-SR04 Echo frente	36
HC-SR04 Echo trás	39
Decisões de projeto
Os TCRT5000 são alimentados em 3,3 V. Em 5 V, o pino D0 sairia em 5 V, acima do limite do ESP32.
O Echo do HC-SR04 sai em 5 V e passa por divisor resistivo (1 kOhm em série, 2 kOhm para GND) antes de entrar no ESP32.
Os sinais usam só o lado esquerdo do ESP32 porque, no protoboard de 400 furos, só esse lado fica acessível. Por isso os dois HC-SR04 compartilham um Trig e o STBY fica fixo em 5 V.
A ponte H é alimentada direto pela bateria (VM), sem passar pelo protoboard, e o LM2596 alimenta só a lógica.
Estrutura do repositório
motor_test/        Programa de teste dos motores (ESP-IDF)
  CMakeLists.txt
  main/
    CMakeLists.txt
    main.c
README.md
Compilar e gravar

Requisitos: ESP-IDF 5.x (testado na 5.5.5), alvo esp32.

cd motor_test
idf.py set-target esp32
idf.py -p COMx build flash monitor

Troque COMx pela porta do ESP32. Também funciona pela extensão ESP-IDF do VS Code, com método de gravação UART.

Cuidados na gravação:

Grave com a chave da bateria desligada. Não alimente o ESP32 por USB e bateria ao mesmo tempo.
Se aparecer "Wrong boot mode detected", segure o botão BOOT durante o "Connecting...".
Se a conexão cair, reduza o baud de gravação para 115200.
Teste de motores

O programa motor_test aguarda 5 s e executa, a 50% de PWM, a sequência: motor esquerdo (frente e ré), motor direito (frente e ré), os dois juntos (frente e ré) e giro no lugar para os dois lados. Execute com as rodas fora do chão. O botão EN repete o teste.

Status
Etapa	Situação
Energia (bateria, chave, LM2596 em 5 V)	Concluída
ESP32 alimentado e gravando via ESP-IDF	Concluída
Ponte H e motores	Montados, teste de motores gravado
Sensores de borda TCRT5000	Em teste
Sensores HC-SR04	Aguardando resistores do divisor de tensão
Chassi final	A fazer
Firmware da estratégia de combate	A fazer
Próximos passos
Concluir o teste dos TCRT5000 e calibrar no dojô.
Montar os divisores de tensão e testar os HC-SR04.
Implementar a estratégia como máquina de estados: evitar a borda, buscar o adversário, atacar.
Fixar tudo no chassi e conferir massa (até 500 g) e dimensões (até 10 x 10 cm).
Testes de combate, relatório e competição final.
