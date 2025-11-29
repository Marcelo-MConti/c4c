# O protocolo

Para o modo de jogo netplay, foi necessário escolher um protocolo para
realizar a comunicação entre as duas instâncias do jogo. O protocolo
implementado é peer-to-peer, baseado em datagrama, usando UDP/IP,
implementa retransmissão e garante que o estado do jogo sempre estará
sincronizado corretamente entre as duas instâncias, assumindo que não
há um jogador ou um MITM (*man-in-the-middle*) mal-intencionado.

## Formato da mensagem

As mensagens usam inteiros *little-endian* de 32 bits, sem sinal. Todas
as mensagens têm pelo menos dois campos: `seq_id` e `type`. `seq_id` é
um identificador sequencial e único para a mensagem dentre todas as
mensagens mandadas pela mesma instância do jogo. `type` identifica o
tipo da mensagem, assumindo um dos seguintes valores:

 - `HANDSHAKE`: contém um campo `rand`, um número gerado aleatoriamente.
 - `MOVE`: contém um campo `col`, o índice da coluna onde foi feita uma jogada.
 - `ACK`: contém um campo `ref`, corresponde ao valor de `seq_id` da mensagem
   cuja recepção foi reconhecida.

## Semântica

Ao iniciar o jogo, ambos os pares mandam uma mensagem do tipo `HANDSHAKE`
simultaneamente. O lado que tiver enviado a mensagem com o maior número
aleatório gerado irá começar primeiro. Se os números gerados forem iguais,
ambos os lados geram novos números aleatórios e enviam mensagens de handshake
novamente.

O valor inicial de `seq_id` é 0. Cada mensagem recebida por um par deve ser
respondida com uma mensagem do tipo `ACK` (exceto mensagens do tipo `ACK`).
Se após um período de 500ms o lado que enviou uma mensagem não receber um *ack*,
deverá retransmitir a mensagem. O valor de `seq_id` é incrementado a cada
mensagem enviada, exceto *acks* e retransmissões.

Para mensagens do tipo `ACK`, `seq_id` deve ser `0xffffffff`. Nenhum lado pode
mandar uma nova mensagem se houver retransmissões pendentes. Ambos os lados
devem ignorar mensagens com saltos não-incrementais no valor de `seq_id`,
exceto mensagens do tipo `ACK`.

Se um par receber uma mensagem com o mesmo valor de `seq_id` que a última
mensagem recebida, deve retransmitir o *ack*.
