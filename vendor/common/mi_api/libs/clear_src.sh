#clear the mijia lib src file 
find . -name "*.c" | grep -v "mi_config.c" | grep -v "./third_party/micro-ecc" | xargs rm -rf
find . -name "*.c" | grep -v "mi_config.c" | grep -v "./third_party/micro-ecc" | xargs rm -rf

::exit
