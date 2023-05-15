import argparse
import random
from typing import List


def generate_string_list(length_of_string: int, length_of_list: int) -> List[str]:
    return [
        "".join(random.choices("0123456789", k=length_of_string))
        for _ in range(length_of_list)
    ]


def write_to_file(file_path: str, data: List[str]) -> None:
    with open(file_path, "w") as f:
        for item in data:
            f.write("%s\n" % item)


def main(
    length_of_string: int, length_of_list: int, file_path1: str, file_path2: str
) -> None:
    list1 = generate_string_list(length_of_string, length_of_list)
    list2 = generate_string_list(length_of_string, length_of_list)

    write_to_file(file_path1, list1)
    write_to_file(file_path2, list2)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("str_length", help="Length of randomly generated string")
    parser.add_argument("out_length", help="Number of strings to generate")
    parser.add_argument("output1", help="Path to the first output file")
    parser.add_argument("output2", help="Path to the second output file")
    args = parser.parse_args()

    main(int(args.str_length), int(args.out_length), args.output1, args.output2)
