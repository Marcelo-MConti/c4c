# Uso de threading

O projeto utiliza uma thread dedicada a IA (simulando um 
jogador). A thread da IA gera movimentos (escolha de colunas)
e se comunica com a outra thread (principal), que é 
responsável pela renderização e pela lógica do jogo.

Ambas as threads disputam pela mesma região de memória, logo, sendo necessário
o uso de um semáforo de exclusão mútua (mutex) como primitivo para sincronizar
os dados compartilhados, como:

1. IA.col : coluna escolhida pela IA

2. IA.has_move: flag que indica se há um movimento pronto para ser consumido

3. IA.running: flag para indicar se a thread deve continuar executando ou
   se deve ser abortada (caso o jogador decida sair do jogo, por exemplo)

