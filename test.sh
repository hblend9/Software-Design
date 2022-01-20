#include <match.h>

#!/bin/bash
function do_test() {
    ./main "$1" "$2"
    student=$?
    echo "$2" | grep "^$1$" > /dev/null
    ref=$?
    if [ $student -ne $ref ]; then
        echo "Failed Test: pattern=\"$1\", text=\"$2\"";
        exit 1;
    fi
}

# Write Your Tests Here
# The do_test function tests if your program gives the 
# same result as the reference implementation on the pattern
# and text provided.

do_test 'h.*e' 'halle'
do_test '*' 'hello'
do_test 'a*da*m' 'aaaadam'
do_test '*' '!'
do_test '*' '1'
do_test 'i*m*iYYYY*' 'iYYY'

echo '-----------------';
echo 'All Tests Passed!';
