# 📘 Como jogar

## 📦 Como instalar

Para executar o jogo corretamente, é necessário possuir alguns componentes instalados no sistema.  
Abaixo estão os requisitos:
### 🔧 Pré-requisitos
- **Compilador C** (GCC ou Clang)
- **Make**
- **Biblioteca ncurses** (essencial para a interface no terminal)
- Terminal compatível com UTF-8(Opcional)

## ▶️ Execução

```bash
# Clonar o repositório
git clone https://gitlab.com/goll72/c4c

# Entrar na pasta
cd c4c

# Compilar
make -j

# Rodar
./c4c
```
## 🎮 Iniciar partida

Ao abrir, o jogo exibirá o menu principal, onde você poderá escolher entre:

**PL VS PL** – dois jogadores no mesmo computador, revezando turnos. 

**PL VS PC** – um jogador contra o computador

**NET PLAY** – Conecta-se com um jogador em outro computador por meio do IP do computador de ambos.

Ao escolher a opção do jogo basta apertar start para iniciar.

### 🕹️ Lógica do Jogo
O Connect 4 é um jogo de estratégia para dois jogadores, que jogam em um tabuleiro vertical de 7 colunas por 6 linhas.

### Resumo do jogo

1. Os jogadores se alternam jogando peças em uma coluna
2. A peça irá cair até o ponto mais baixo possível na coluna
3. O objetivo é alinhar 4 peças da mesma cor em uma das seguintes direções:
    * Horizontal
    * Vertical
    * Diagonal(Ambas as direções)

Ao alinhar 4 peças iguais, o jogo sinaliza a vitória fazendo as peças piscarem(Blink effect) e 
escrevendo a mensagem de vitória e término da partida.

### Comandos do jogo

- **Mover a seta:**  
  - ⬅️ `Esquerda` → Move a seta para a coluna anterior  
  - ➡️ `Direita` → Move a seta para a próxima coluna  
  - ⤒ `Home` → Move a seta para a primeira coluna  
  - ⤓ `End` → Move a seta para a última coluna

- **Jogar a peça:**  
  - ⏎ `Enter` → Coloca a peça do jogador na coluna selecionada

- **Desistir / Sair do jogo:**  
  - ❌ `Ctrl + C` → Interrompe a partida e volta ao menu principal

- **Redesenhar a tela:**  
  - 🔄 `Resize` → A interface será ajustada automaticamente caso a janela seja redimensionada
