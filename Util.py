import csv

def generate_csv_with_out_heads(content,filePath):
        with open(filePath, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerows(content)
