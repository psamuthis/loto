import matplotlib.pyplot as plt
from collections import Counter

def analyze_loto_distribution(file_path):
    all_numbers = []

    try:
        with open(file_path, 'r') as f:
            for line in f:
                nums = [int(n) for n in line.split() if n.strip()]
                all_numbers.extend(nums)
    except FileNotFoundError:
        print("Error: results.txt not found. Run the C program first!")
        return

    counts = Counter(all_numbers)
    x = list(range(1, 50))
    y = [counts.get(i, 0) for i in x]

    plt.figure(figsize=(12, 6))
    plt.bar(x, y, color='skyblue', edgecolor='navy')
    plt.axhline(y=sum(y)/49, color='r', linestyle='--', label='Theoretical Average')
    plt.title(f'Distribution of Loto Numbers ({len(all_numbers)//6} draws)')
    plt.xlabel('Loto Number')
    plt.ylabel('Frequency')
    plt.xticks(x)
    plt.legend()
    plt.show()

analyze_loto_distribution('./output-data/results.txt')