#ifndef _LOGO_H_
#define _LOGO_H_
/*
  O arquivo define a logo inicial do jogo que pode ser gerada
  em ASCII ou UNICODE. Cada string representa uma linha do jogo 

 */
#ifdef C4C_ASCII
static const char *logo[] = {
      "######+ ##+ ##+ ######+" "\n",
      "##+---+ ##| ##| ##+---+" "\n",
      "##|     ######| ##|    " "\n",
      "######+ +---##| ######+" "\n",
      "+-----+     +-+ +-----+" "\n"
};
#else
static const char *logo[] = {
    u8"██████┐ ██┐ ██┐ ██████┐" "\n",
    u8"██┌───┘ ██│ ██│ ██┌───┘" "\n",
    u8"██│     ██████│ ██│    " "\n",
    u8"██████┐ └───██│ ██████┐" "\n",
    u8"└─────┘     └─┘ └─────┘" "\n"
};
#endif

#endif /* _LOGO_H_ */
