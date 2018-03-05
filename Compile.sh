echo "building shared --------------------"
cd ./Source/Share
for f in *.cpp; do
    echo "building $f"
    g++ -g3 -Wall -std=c++14 -c $f
    mv "$(basename $f .cpp).o" "../../$(basename "Share__$f" .cpp).o"
done
cd -




echo ""
echo "building client --------------------"
cd ./Source/Client
for f in *.cpp; do
    echo "building $f"
    g++ -g3 -Wall -std=c++14 -c $f
    mv "$(basename $f .cpp).o" "../../$(basename "Client__$f" .cpp).o"
done
cd -

echo "linking"
g++ ./Client__*.o ./Share__*.o -o client -lsfml-graphics -lsfml-network -lsfml-window -lsfml-system -lBox2D -pthread




echo ""
echo "building server --------------------"
cd ./Source/Server
for f in *.cpp; do
    echo "building $f"
    g++ -g3 -Wall -std=c++14 -c $f  
    mv "$(basename $f .cpp).o" "../../$(basename "Server__$f" .cpp).o"
done
cd -

echo "linking"
g++ ./Server__*.o ./Share__*.o -o server -lsfml-graphics -lsfml-network -lsfml-window -lsfml-system -lBox2D -pthread


