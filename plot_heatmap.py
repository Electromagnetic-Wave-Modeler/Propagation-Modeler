import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

def plot_wifi_heatmap(csv_file, output_image):
    # Lire le fichier CSV
    df = pd.read_csv(csv_file)

    # Supprimer les espaces dans les noms de colonnes
    df.columns = df.columns.str.strip()

    # Vérifier les colonnes requises
    if not {'X', 'Y', 'Power_dBm'}.issubset(df.columns):
        print("❌ Format de fichier CSV incorrect, colonnes manquantes : X, Y, Power_dBm")
        print("📄 Colonnes détectées :", df.columns)
        return

    # Conversion des colonnes en float
    df['X'] = df['X'].astype(float)
    df['Y'] = df['Y'].astype(float)
    df['Power_dBm'] = df['Power_dBm'].astype(float)

    # Remplacer -inf par la valeur minimale valide
    valid_power = df[df['Power_dBm'] > -np.inf]
    if not valid_power.empty:
        min_valid_power = valid_power['Power_dBm'].min()
    else:
        min_valid_power = -100  # Valeur par défaut si toutes les valeurs sont -inf
    df['Power_dBm'].replace(-np.inf, min_valid_power, inplace=True)

    # Créer une grille de coordonnées
    x_values = np.sort(df["X"].unique())
    y_values = np.sort(df["Y"].unique())[::-1]  # Inverser pour l'axe Y

    power_grid = np.full((len(y_values), len(x_values)), np.nan)

    # Remplir la grille
    for _, row in df.iterrows():
        x_idx = np.where(x_values == row["X"])[0][0]
        y_idx = np.where(y_values == row["Y"])[0][0]
        power_grid[y_idx, x_idx] = row["Power_dBm"]

    # Définir les limites de la couleur
    vmin = np.nanmin(power_grid)
    vmax = np.nanmax(power_grid)
    power_grid = np.nan_to_num(power_grid, nan=vmin)

    # Créer la heatmap
    plt.figure(figsize=(10, 8))
    ax = sns.heatmap(
        power_grid,
        xticklabels=x_values,
        yticklabels=y_values,
        cmap="RdYlGn",
        center=0,
        cbar_kws={'label': 'Puissance (dBm)'},
        linewidths=0.1,
        linecolor='gray',
        vmin=vmin,
        vmax=vmax
    )
    ax.invert_yaxis()  # Inversion de l'axe Y pour correspondre aux coordonnées

    plt.title("Carte thermique du signal WiFi (dBm)")
    plt.xlabel("Coordonnée X")
    plt.ylabel("Coordonnée Y")
    plt.xticks(rotation=45)
    plt.yticks(rotation=0)

    # Sauvegarder l'image
    plt.savefig(output_image, dpi=300, bbox_inches='tight')
    print(f"✅ Carte thermique sauvegardée : {output_image}")
    plt.close()

# Exécuter la fonction
plot_wifi_heatmap("heatmap.csv", "wifi_heatmap.png")