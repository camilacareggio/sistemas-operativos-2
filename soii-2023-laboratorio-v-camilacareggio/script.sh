for i in {1..100}
do
	echo 
	curl -X POST http://localhost:8537/increment
    echo
    curl -X GET http://localhost:8537/imprimir
done