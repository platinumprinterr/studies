import tkinter as tk
from tkinter import messagebox
import secrets
import string

def generate_password():
    try:
        length = int(length_entry.get())
    except ValueError:
        messagebox.showerror("Invalid Input", "Please enter a valid number for length.")
        return

    characters = string.ascii_letters
    if digits_var.get():
        characters += string.digits
    if specials_var.get():
        characters += string.punctuation

    if not characters:
        messagebox.showwarning("No Option Selected", "Select at least one character type.")
        return

    password = ''.join(secrets.choice(characters) for _ in range(length))
    password_output.delete(0, tk.END)
    password_output.insert(0, password)

def copy_to_clipboard():
    password = password_output.get()
    if password:
        root.clipboard_clear()
        root.clipboard_append(password)
    else:
        messagebox.showwarning("Empty", "No password to copy.")

root = tk.Tk()
root.title("Simple Password Generator")

tk.Label(root, text="Password Length:").grid(row=0, column=0, padx=5, pady=5, sticky="w")
length_entry = tk.Entry(root, width = 5)
length_entry.insert(0, "20")
length_entry.grid(row=0, column=1, padx=5, pady=5, sticky ="w")

digits_var = tk.BooleanVar(value=True)
specials_var = tk.BooleanVar(value=True)
tk.Checkbutton(root, text="Include Digits", variable=digits_var).grid(row=1, column=0, columnspan=2, sticky="w", padx=5)
tk.Checkbutton(root, text="Include Special Characters", variable=specials_var).grid(row=2, column=0, columnspan=2, sticky="w", padx=5)

tk.Button(root, text="Generate Password", command=generate_password).grid(row=3, column=0, columnspan=2, pady=10)

tk.Label(root, text="Generated Password:").grid(row=4, column=0, padx=5, pady=(20, 5), sticky="w")
password_output = tk.Entry(root, width=30)
password_output.grid(row=4, column=1, padx=(5, 10), pady=(20, 5), sticky ="w")

tk.Button(root, text="Copy to Clipboard", command=copy_to_clipboard).grid(row=5, column=0, columnspan=2, pady=10)

root.mainloop()
