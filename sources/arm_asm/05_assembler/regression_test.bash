#!/bin/bash
function check () {
    FILE_NAME=$1    
    ./a.out  "./test/test_input/${FILE_NAME%.txt}.ks" | diff  -u "./test/test_expect/${FILE_NAME}" - >&2
    if [ $? = 0 ]; then
        echo "ok"
    else
        echo $FILE_NAME >&2
        echo "ng"
    fi
}

RESULT=$(for file_name in $(ls ./test/test_expect); do
    check $file_name
done)

echo "$(echo "$RESULT" | wc -l | tr -d ' ') tests: $(echo $RESULT | grep -o 'ok' | wc -l| tr -d ' ') ok, $(echo $RESULT | grep -o 'ng' | wc -l| tr -d ' ') ng"