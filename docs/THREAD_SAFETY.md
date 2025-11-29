# Uso de threading

O jogo utiliza duas threads: a thread principal, responsável pela
renderização, e uma thread auxiliar, que se encarrega de implementar o
[protocolo](PROTOCOL.md) de comunicação no modo netplay. As duas threads
compartilham uma região de memória, usada para sinalizar que:

 - há uma jogada que foi feita localmente que deve ser transmitida
   para a outra instância do jogo; OU

 - há uma jogada que foi recebida pela rede que deve ser processada
   localmente.

Essa região de memória é protegida por um semáforo de exclusão mútua
(mutex) e guarda os seguintes dados:

 - `col (int32_t)`: assume o valor `-1` se não houver uma jogada pendente.
   caso contrário, guarda a coluna onde foi feita uma jogada.

 - `remote (bool)`: indica se a jogada pendente é uma jogada local, que
   deve ser enviada, ou uma jogada remota, que foi recebida e deve ser
   processada.
