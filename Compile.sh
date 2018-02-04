for f in ./*.cpp; do
    echo "building $f"
    g++ -c $f 
done

echo "linking game"
g++ ./*.o -o game -lsfml-graphics -lsfml-window -lsfml-system -lBox2D
