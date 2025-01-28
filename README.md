# Controlador de Demanda no ESP32

[comment]: # (Trabalho da faculdade)

Este programa é um controlador de demanda utilizando um módulo RTC DS3231 para tempo real e um módulo Relé.

O programa precisa de um sensor de LED para conseguir pegar os pulsos enviados pelo medidor de energia para então ser enviado para o ESP32. 
Dependendo do intervalo de pulsos no código e a hora mínima para controlar a demanda, o relé vai ser ativada ou não.

Além disso, também é utilizado uma biblioteca Modbus TCP/IP para fazer conexão com o SCADA-LTS para supervisionado do estado do relé e o intervalo de pulsos.
