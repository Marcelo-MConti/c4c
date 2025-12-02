# Uso de threading

O projeto utiliza uma thread dedicada a IA (simulando um 
jogador), a Thread da IA gera movimentos (escolha de colunas)
e se comunicação com a segunda thread (principal) que é 
responsável pela renderização.

Ambas as threads disputam as mesmas regiões de memoria, logo, sendo necessario o uso de um mutex como primitivo para sincronizar os campos compartilhados como:

1) IA.col : coluna escolhida pela IA
2) IA.has_move: flag que indica se há um movimento pronto para ser consumido
3) IA.running: flag para indicar se a thread deve continuar executando

