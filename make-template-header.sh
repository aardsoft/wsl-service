#!/bin/sh

OUT=build/wsl-templates.h

write_template(){
    template_string=`grep -v ' *#' $2 | tr '\n' ';' | sed -E "s/ +/ /g"`;
    echo "#define $1 \"/bin/bash -c '$template_string'\"" >> ${OUT}
}

echo '#ifndef _WSL_TEMPLATES_H' > ${OUT}
echo '#define _WSL_TEMPLATES_H' >> ${OUT}
write_template BASH_START_TEMPLATE start-service-template.sh
write_template BASH_STOP_TEMPLATE stop-service-template.sh
echo '#endif' >> ${OUT}
