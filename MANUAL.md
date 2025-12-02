c4c
===

c4c é um jogo de Connect 4 no terminal escrito em C, usando ncurses.

## 📘 Como jogar

### 🎮 Iniciar partida

Ao abrir, o jogo exibirá o menu principal, onde você poderá escolher entre:

**PL VS PL** – dois jogadores no mesmo computador, revezando turnos. 

**PL VS PC** – um jogador contra o computador

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


## 📦 Compilação e instalação

As seguintes dependências são necessárias para compilar o jogo:

 - Um compilador C que suporte GNU C11 (gcc ou clang)
 - GNU make
 - ncurses
 - GNU gettext

Para compilar o jogo, basta rodar `make` ou `make all`. As variáveis
`CFLAGS`, `CPPFLAGS`, `LDLIBS`, `BUILD`, `PREFIX` e `DESTDIR` são
suportadas (embora não seja necessário especificá-las, geralmente),
e têm o significado usual. Além das variáveis usuais, `ASCII` e `NOCOLOR`
podem ser usadas para forçar o jogo a usar apenas caracteres ASCII para
renderizar elementos visuais (em vez de Unicode) e desativar cores,
respectivamente, ex. `make ASCII=1 NOCOLOR=1`.

```bash
# Clonar o repositório
git clone https://gitlab.com/goll72/c4c

# Entrar na pasta
cd c4c

# Compilar
make -j

```

Após compilar, o jogo pode ser rodado diretamente:

```
$ ./c4c
```

É possível instalar o jogo (por padrão em `/usr/local`, pode ser
alterado usando `PREFIX` e `DESTDIR`), rodando `make install`.
Após instalar o jogo, a variável de ambiente `LANGUAGE` pode ser
usada para que a interface de usuário seja traduzida para português:

```
$ LANGUAGE=pt c4c  
```

> [!NOTE]
>
> O jogo foi testado em Linux. <!-- ... -->
