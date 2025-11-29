c4c
===

c4c é um jogo de Connect 4 no terminal escrito em C, usando ncurses.

## Como jogar

<!-- ... -->

## Compilação e instalação

As seguintes dependências são necessárias para compilar o jogo:

 - Um compilador C que suporte GNU C11 (gcc ou clang)
 - GNU make
 - ncurses
 - GNU gettext

Para compilar o jogo, basta rodar `make` ou `make all`. As variáveis
`CFLAGS`, `CPPFLAGS`, `LDLIBS`, `BUILD`, `PREFIX` e `DESTDIR` são
suportadas, e têm o significado usual. Após compilar, o jogo pode ser
rodado diretamente:

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
