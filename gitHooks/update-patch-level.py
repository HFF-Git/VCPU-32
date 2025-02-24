import re
import sys

# Function to increment the PATCH_ LEVEL number
def increment_patch_level(line):
    # Regular expression to match "const int PATCH_ LEVEL = <number>"
    match = re.search(r'const\s+int\s+PATCH_\s*LEVEL\s*=\s*(\d+);', line)
    
    if match:
        # Extract the current number
        current_value = int(match.group(1))
        # Increment the number
        incremented_value = current_value + 1
        # Replace the old value with the new incremented value
        updated_line = line.replace(str(current_value), str(incremented_value))
        return updated_line
    return line

# Function to update the file
def update_patch_level_in_file(file_path):
    try:
        # Open the file for reading
        with open(file_path, 'r') as file:
            lines = file.readlines()
        
        # Open the file for writing
        with open(file_path, 'w') as file:
            for line in lines:
                # Increment PATCH_ LEVEL if needed
                updated_line = increment_patch_level(line)
                file.write(updated_line)
        
        print(f"File '{file_path}' has been updated successfully.")
    
    except FileNotFoundError:
        print(f"Error: The file '{file_path}' was not found.")
    except Exception as e:
        print(f"Error: {e}")

# Main function
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python increment_patch_level.py <file_path>")
    else:
        file_path = sys.argv[1]
        update_patch_level_in_file(file_path)