#!/bin/sh

exec 3>logo.h

cat << EOF >&3
#ifndef _LOGO_H_
#define _LOGO_H_

static const char *logo[] = {
EOF

OD=$(od -b -v logo | sed -E 's/^[0-9]{7}//g')

printf '    "' >&3

for i in $OD; do
    if [ "$i" = 012 ]; then
        printf '",\n'
    else
        [ "$last" = 012 ] && printf '    "'
        printf '\\%s' "$i"
    fi

    last=$i
done >&3

cat << EOF >&3
};

#endif /* _LOGO_H_ */
EOF
