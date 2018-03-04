for f in ./Source/*.cpp; do
    echo "building $f"
    g++ -g3 -Wall -std=c++14 -c $f 
done

echo "linking game"
g++ ./*.o -o game -lsfml-graphics -lsfml-network -lsfml-window -lsfml-system -lBox2D
