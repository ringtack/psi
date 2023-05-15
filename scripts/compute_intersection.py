import argparse


def read_file(file_path: str) -> set:
    with open(file_path, "r") as file:
        return set(line.strip() for line in file)


def write_to_file(file_path: str, data: set) -> None:
    with open(file_path, "w") as file:
        for item in data:
            file.write("%s\n" % item)


def find_common_lines(file_path1: str, file_path2: str, output_file_path: str) -> None:
    lines1 = read_file(file_path1)
    lines2 = read_file(file_path2)

    common_lines = lines1.intersection(lines2)

    write_to_file(output_file_path, common_lines)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file1", help="Path to the first file")
    parser.add_argument("file2", help="Path to the second file")
    parser.add_argument("output_file", help="Path to the output file")
    args = parser.parse_args()

    find_common_lines(args.file1, args.file2, args.output_file)
