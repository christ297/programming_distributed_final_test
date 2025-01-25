import tkinter as tk
from tkinter import ttk
import subprocess
import threading


class MultiTerminalApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Multi-Terminal Interface")
        self.root.geometry("1200x800")
        self.root.resizable(False, False)

        # Configuration des colonnes
        self.rows = 2
        self.cols = 3
        self.current_terminals = 0

        # Cadre pour les entrées utilisateur
        self.input_frame = ttk.Frame(self.root)
        self.input_frame.pack(side="top", fill="x", padx=5, pady=5)

        ttk.Label(self.input_frame, text="Commande:").pack(side="left", padx=5)
        self.command_entry = ttk.Entry(self.input_frame, width=50)
        self.command_entry.pack(side="left", padx=5)

        add_button = ttk.Button(self.input_frame, text="Ajouter Terminal", command=self.add_terminal)
        add_button.pack(side="left", padx=5)

        # Cadre pour les terminaux
        self.terminal_frame = ttk.Frame(self.root)
        self.terminal_frame.pack(expand=True, fill="both", padx=5, pady=5)

        # Stockage des terminaux
        self.terminals = {}

    def add_terminal(self):
        """Ajoute un terminal à la grille."""
        command = self.command_entry.get().strip()
        if not command:
            self.show_error("Veuillez fournir une commande à exécuter.")
            return

        if self.current_terminals >= self.rows * self.cols:
            self.show_error("Limite de terminaux atteinte.")
            return

        # Calculer la position dans la grille
        row = self.current_terminals // self.cols
        col = self.current_terminals % self.cols

        # Créer un cadre pour le terminal
        terminal_frame = ttk.Frame(self.terminal_frame, borderwidth=2, relief="solid")
        terminal_frame.grid(row=row, column=col, padx=5, pady=5, sticky="nsew")

        # Ajouter un widget Text pour afficher la sortie
        text_widget = tk.Text(terminal_frame, wrap="word", bg="black", fg="white", font=("Courier", 8))
        text_widget.pack(expand=True, fill="both", padx=5, pady=5)
        text_widget.insert("1.0", f"Lancement de la commande : {command}\n")

        self.terminals[self.current_terminals] = {"frame": terminal_frame, "text_widget": text_widget}

        # Lancer le programme dans un thread séparé
        threading.Thread(target=self.run_program, args=(command, text_widget), daemon=True).start()

        self.current_terminals += 1

    def run_program(self, command, text_widget):
        """Exécute un programme et redirige ses sorties vers un widget Text."""
        try:
            process = subprocess.Popen(
                command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
            )

            # Lire les sorties et les afficher
            while True:
                output = process.stdout.readline()
                if output == "" and process.poll() is not None:
                    break
                text_widget.insert("end", output)
                text_widget.see("end")  # Scroll automatique vers le bas

            # Afficher les erreurs si présentes
            err_output = process.stderr.read()
            if err_output:
                text_widget.insert("end", "\n[ERROR]\n" + err_output, "error")
                text_widget.tag_config("error", foreground="red")

        except Exception as e:
            text_widget.insert("end", f"Erreur: {str(e)}\n", "error")
            text_widget.tag_config("error", foreground="red")

    def show_error(self, message):
        """Affiche un message d'erreur dans une boîte de dialogue."""
        error_window = tk.Toplevel(self.root)
        error_window.title("Erreur")
        error_window.geometry("300x100")
        ttk.Label(error_window, text=message, foreground="red").pack(expand=True, pady=20)
        ttk.Button(error_window, text="Fermer", command=error_window.destroy).pack()


if __name__ == "__main__":
    root = tk.Tk()
    app = MultiTerminalApp(root)
    root.mainloop()
