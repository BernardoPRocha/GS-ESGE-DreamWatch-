# DreamWatch

## Descrição do Problema de Saúde
O projeto DreamWatch aborda questões relacionadas à qualidade do sono, monitorando movimentos, temperatura e umidade durante o sono. Uma boa qualidade de sono é essencial para a saúde geral e bem-estar, e este projeto visa fornecer insights sobre os padrões de sono.

## Visão Geral da Solução Proposta
O DreamWatch utiliza um ESP32, sensor PIR (Passive Infrared), sensor de temperatura e umidade DHT22, e um display TM1637 para monitorar o sono. Ele registra dados como a média de temperatura, a média de umidade, os movimentos durante o sono, e fornece uma análise detalhada ao final do período de sono.

Além disso, o projeto se integra a um servidor MQTT, permitindo que os dados do sono sejam enviados para uma plataforma de IoT para análise remota ou notificações.

## Configuração e Execução
1. **Configuração do Hardware:**
   - Conecte o sensor PIR, sensor DHT22 e o display TM1637 ao ESP32 conforme as instruções do código.
   - Certifique-se de ter um servidor MQTT disponível e atualize as configurações de Wi-Fi e MQTT no código.

2. **Configuração do Software:**
   - Abra o código no Arduino IDE ou em um ambiente de desenvolvimento de sua escolha.
   - Instale as bibliotecas necessárias, se ainda não estiverem instaladas (DHT, TM1637, WiFi, PubSubClient, etc.).

3. **Execução:**
   - Compile e carregue o código para o ESP32.
   - Monitore a saída serial para obter informações sobre o início e o fim do sono, bem como dados do sono.

## Estrutura do Repositório
- `/main.ino`: Contém os códigos-fonte do projeto.

## Simulação no Wokwi
Para visualizar uma simulação do projeto, [clique aqui](https://wokwi.com/projects/381479333854213121).





