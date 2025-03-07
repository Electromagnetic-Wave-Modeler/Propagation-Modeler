import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

def plot_wifi_heatmap(csv_file, output_image):
    # Lire le fichier CSV sans en-tête, comme une matrice de valeurs
    try:
        power_grid = np.loadtxt(csv_file, delimiter=',')
        print(f"✅ Fichier CSV chargé avec succès. Dimensions: {power_grid.shape}")
    except Exception as e:
        print(f"❌ Erreur lors du chargement du fichier CSV: {e}")
        return

    # Vérifier si des données ont été chargées
    if power_grid.size == 0:
        print("❌ Aucune donnée n'a été chargée depuis le fichier CSV")
        return

    # Définir les limites de la couleur
    vmin = np.nanmin(power_grid)
    vmax = np.nanmax(power_grid)
    
    # Remplacer les NaN par la valeur minimale
    power_grid = np.nan_to_num(power_grid, nan=vmin)

    # Créer des étiquettes d'axes basées sur les indices
    y_indices = np.arange(power_grid.shape[0])
    x_indices = np.arange(power_grid.shape[1])

    # Créer la heatmap
    plt.figure(figsize=(10, 8))
    ax = sns.heatmap(
        power_grid,
        cmap="RdYlGn",
        center=(vmin + vmax) / 2,  # Centre de la palette de couleurs
        cbar_kws={'label': 'Puissance (dBm)'},
        linewidths=0.1,
        linecolor='gray',
        vmin=vmin,
        vmax=vmax
    )
    
    # Configurer les axes et le titre
    plt.title("Carte thermique du signal WiFi (dBm)")
    plt.xlabel("Position X")
    plt.ylabel("Position Y")
    
    # Réduire le nombre de ticks si nécessaire pour les grands ensembles de données
    if len(x_indices) > 20:
        plt.xticks(np.linspace(0, len(x_indices)-1, 10).astype(int), 
                  np.linspace(0, len(x_indices)-1, 10).astype(int))
    if len(y_indices) > 20:
        plt.yticks(np.linspace(0, len(y_indices)-1, 10).astype(int), 
                  np.linspace(0, len(y_indices)-1, 10).astype(int))
    
    # Inverser l'axe Y pour avoir l'origine en haut à gauche (convention cartographique)
    plt.gca().invert_yaxis()

    # Sauvegarder l'image
    plt.savefig(output_image, dpi=300, bbox_inches='tight')
    print(f"✅ Carte thermique sauvegardée : {output_image}")
    plt.close()

# Exécuter la fonction
plot_wifi_heatmap("heatmap.csv", "wifi_heatmap.png")