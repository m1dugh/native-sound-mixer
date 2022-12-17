#!/bin/sh


if [ $# -ne 2 ];then
    echo "Usage: format.sh <path> <config>"
    exit 1
fi

path=$1
config=$2

if [ ! -f "$config" ];then
    echo "$config is not a clang-format config file"
    exit 1
fi

if [ ! -d "$path" ];then
    echo "$path is not a folder"
    exit 1
fi

which find > /dev/null
exit_code=$?
if [ $exit_code -ne 0 ];then
    echo "could not locate 'find'"
    exit 1
fi

which clang-format > /dev/null
exit_code=$?
if [ $exit_code -ne 0 ];then
    echo "could not locate 'clang-format'"
    exit 1
fi

for f in `find $path -type f | grep -E "\.(h|c)pp$"`;do
    echo "formatting $f ..."
    clang-format -i "$f" --style=file:"$config" || exit 1
done

echo "Formatted all files"

