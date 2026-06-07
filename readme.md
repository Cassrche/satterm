# SatTherm G

Sistema embarcado autônomo para monitoramento térmico e energético de um nanossatélite, desenvolvido para a disciplina **Edge Computing & Computer Systems**.

O projeto simula um subsistema aeroespacial de missão crítica, capaz de monitorar temperatura e bateria, processar os dados localmente e acionar alarmes ou atuadores de forma automática, sem depender de conexão externa.

---

## 1. Contexto do Projeto

Em dispositivos aeroespaciais, como nanossatélites, o acesso físico humano é limitado ou inexistente após o lançamento. Por isso, o sistema embarcado precisa ser capaz de tomar decisões locais de forma autônoma.

O **SatTherm ** foi projetado para atuar sobre dois riscos principais:

- **Temperatura inadequada**, que pode danificar componentes eletrônicos;
- **Baixo nível de bateria**, que exige desligamento de cargas não críticas para preservar a missão.

---

## 2. Objetivo

Construir um sistema autônomo capaz de:

- Monitorar temperatura interna;
- Monitorar nível de bateria;
- Exibir telemetria em display LCD;
- Acionar LEDs de status;
- Emitir alertas sonoros;
- Acionar aquecimento ou resfriamento;
- Abrir uma aleta térmica simulada por servo motor;
- Desligar carga não crítica em caso de bateria baixa;
- Operar sem uso de `delay()`, utilizando `millis()`.

---

## 3. Componentes Utilizados

| Componente | Função |
|---|---|
| Arduino Uno | Unidade principal de processamento |
| Sensor TMP36 | Leitura da temperatura |
| Potenciômetro | Simulação do nível de bateria |
| Display LCD 16x2 | Telemetria local |
| Potenciômetro de contraste | Ajuste visual do LCD |
| LED verde | Estado normal |
| LED amarelo | Estado de atenção ou bateria crítica |
| LED vermelho | Estado crítico ou falha |
| Piezo/Buzzer | Alarme sonoro |
| Servo motor | Aleta térmica simulada |
| LED laranja | Aquecedor simulado |
| LED azul | Resfriador simulado |
| LED branco | Carga não crítica |
| Resistores 220Ω | Proteção dos LEDs e backlight do LCD |
| Protoboard e jumpers | Montagem do circuito |

---

## 4. Mapeamento de Pinos

| Pino Arduino | Componente |
|---|---|
| A0 | Sensor de temperatura TMP36 |
| A1 | Potenciômetro da bateria |
| A2 | LCD D6 |
| A3 | LCD D7 |
| D2 | LED verde |
| D3 | LED amarelo |
| D4 | LED vermelho |
| D5 | Piezo/Buzzer |
| D6 | Aquecedor simulado |
| D7 | Resfriador simulado |
| D8 | Carga não crítica |
| D9 | Servo motor |
| D10 | LCD RS |
| D11 | LCD E |
| D12 | LCD D4 |
| D13 | LCD D5 |

---

## 5. Ligação do LCD

| Pino do LCD | Ligação |
|---|---|
| VSS / GND | GND |
| VDD / VCC | 5V |
| VO | Pino central do potenciômetro de contraste |
| RS | D10 |
| RW | GND |
| E | D11 |
| D4 | D12 |
| D5 | D13 |
| D6 | A2 |
| D7 | A3 |
| Anode / LED+ | 5V com resistor de 220Ω |
| Cathode / LED- | GND |

---

## 6. Estados do Sistema

| Estado | Condição | Ação |
|---|---|---|
| Normal | Temperatura segura e bateria acima de 50% | LED verde ligado e carga ativa |
| Atenção | Temperatura em faixa de atenção ou bateria entre 6% e 50% | LED amarelo ligado |
| Temperatura Alta | Temperatura acima de 45 °C | LED vermelho, buzzer, resfriador e servo aberto |
| Temperatura Baixa | Temperatura abaixo de 0 °C | LED vermelho, buzzer e aquecedor |
| Bateria Crítica | Bateria entre 1% e 5% | LED amarelo e carga não crítica desligada |
| Bateria Nula | Bateria em 0% | LED vermelho, buzzer e carga não crítica desligada |
| Falha de Sensor | Leitura fora da faixa válida | LED vermelho, alerta e modo seguro |

---

## 7. Regras de Atuação

- Se a temperatura estiver acima de 45 °C, o sistema entra em estado de **temperatura alta**.
- Se a temperatura estiver abaixo de 0 °C, o sistema entra em estado de **temperatura baixa**.
- Se a bateria estiver entre 1% e 5%, o sistema entra em **bateria crítica**.
- Se a bateria estiver em 0%, o sistema entra em **bateria nula**.
- Se a bateria estiver baixa, a carga não crítica é desligada.
- Se houver falha de leitura, o sistema mantém o último valor válido e registra erro em contador RAM.
- O sistema não utiliza `delay()` no loop principal.

---

## 8. Critérios Técnicos Atendidos

| Requisito | Status |
|---|---|
| Dois sensores ou variáveis monitoradas | Atendido |
| Display LCD obrigatório | Atendido |
| LEDs de alerta visual | Atendido |
| Buzzer para alerta sonoro | Atendido |
| Atuador autônomo | Atendido |
| Servo motor | Atendido |
| Temporização com `millis()` | Atendido |
| Sem `delay()` no loop principal | Atendido |
| Validação de leituras | Atendido |
| Diagnóstico via Serial Monitor | Atendido |
| Simulação no Tinkercad | Atendido |

---

## 9. Como Executar

1. Abrir o circuito no Tinkercad.
2. Conferir as ligações conforme o mapeamento de pinos.
3. Colar o código no editor em modo **Text**.
4. Iniciar a simulação.
5. Abrir o **Serial Monitor** em `9600 baud`.
6. Alterar o sensor de temperatura e o potenciômetro da bateria para testar os estados.

---

## 10. Cenários de Teste

| Cenário | Entrada | Resultado Esperado |
|---|---|---|
| Operação normal | Temperatura entre 10 °C e 35 °C e bateria acima de 50% | LED verde e status normal |
| Atenção térmica | Temperatura entre 35 °C e 45 °C | LED amarelo |
| Temperatura alta | Temperatura acima de 45 °C | LED vermelho, buzzer, servo aberto e resfriador ligado |
| Temperatura baixa | Temperatura abaixo de 0 °C | LED vermelho, buzzer e aquecedor ligado |
| Bateria crítica | Bateria entre 1% e 5% | LED amarelo e carga não crítica desligada |
| Bateria nula | Bateria em 0% | LED vermelho, buzzer e carga não crítica desligada |
| Falha de sensor | Temperatura fora da faixa plausível | Estado de falha e modo seguro |

---

## 11. Observação sobre EEPROM

No protótipo do Tinkercad, os contadores de erro foram mantidos em memória RAM por compatibilidade do simulador. Em uma implementação física com Arduino Uno, esses contadores podem ser persistidos com `EEPROM.h`.

---

## 12. Conclusão

O **SatTherm ** demonstra uma aplicação de Edge Computing em um cenário aeroespacial, realizando leitura de sensores, tomada de decisão local e atuação autônoma.

O sistema opera de forma offline, exibe telemetria local no LCD e responde automaticamente a situações críticas, simulando o comportamento esperado de um subsistema embarcado de missão crítica.

