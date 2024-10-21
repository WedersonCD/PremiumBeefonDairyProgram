import csv

def generate_csv_with_out_heads(content,file_path):
        with open(file_path, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(content)

def get_file_content(file_path):
       with open(file_path, mode='r') as file:
        reader = csv.reader(file)
        
        # Iterate through the rows and append to the content list
        for row in reader:
            return row
