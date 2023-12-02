#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#pragma region Definition
#define repeat(_varName, _nbTime) for(int _varName = 0, nb_time = (int)(_nbTime); _varName < nb_time; _varName++)
#define unused(x) (void)(x)

#pragma warning(push)
// Faux positif 
#pragma warning(disable:6385)
// Faux positif sur la lecture hors tableau é cause de la nature du programme avec plein de tableaux dont les indices sont lu depuis d'autre tableaux
#pragma warning(disable:6386)
// La taille de certaine structure est trop grande, mais on ne veut pas d'allocations dynamique
#pragma warning(disable:6262)


using namespace std;
#pragma endregion

#pragma region Structures

const int nMaxClient = 180;

typedef struct probleme
{
	// Nom du probleme
	string nom;
	// Indice de ville du dépot (souvent 0)
	int depot;
	// Nombre de ville, dépot compris
	int nb_ville;
	int dist[nMaxClient][nMaxClient];
	int qte[nMaxClient];
} probleme;


typedef struct solution
{
	int itineraire[nMaxClient + 2];
	union
	{
		// 2 noms différents pour la même variables
		int cout;
		int metres;
	};
} solution;
#pragma endregion

#pragma region Aleatoire
int rand_uni(int min, int max)
{
	return min + rand() % (max - min + 1);
}

float rand_float(float mini, float maxi)
{
	return (rand() / (float)RAND_MAX) * (maxi - mini) + mini;
}
#pragma endregion

#pragma region Heuristiques
void plus_proche_voisin(probleme& p, solution& s)
{
	s.cout = 0;
	int i;

	int ville_rest[nMaxClient];
	// On exclus le dépot qui est compris dans le nb_ville
	int nb_ville_rest = p.nb_ville - 1;

	for (i = 1; i < p.nb_ville; i++)
	{
		ville_rest[i - 1] = i;
	}

	s.itineraire[0] = p.depot;
	for (i = 1; i < p.nb_ville; i++)
	{
		int dist_min = 99999999;
		int idx_ville_min = 0;
		for (int j = 0; j < nb_ville_rest; j++)
		{
			int dist = p.dist[s.itineraire[i - 1]][ville_rest[j]];
			if (dist_min > dist && dist != 0)
			{
				dist_min = dist;
				idx_ville_min = j;
			}
		}
		s.itineraire[i] = ville_rest[idx_ville_min];

		nb_ville_rest--;
		ville_rest[idx_ville_min] = ville_rest[nb_ville_rest];
		s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];

	}
	s.itineraire[i] = p.depot;
	s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];
}

typedef struct ville_dist
{
	int dist;
	int ville;
	int idx_rest;
} ville_dist;

const int taille_liste_plus_proche = 6;
void inserer_trie(ville_dist liste[], ville_dist v)
{
	int i = taille_liste_plus_proche - 1;
	if (liste[i].dist > v.dist)
	{
		liste[i] = v;
		i--;
		while (i >= 0 && liste[i].dist > liste[i + 1].dist)
		{
			ville_dist tmp = liste[i];
			liste[i] = liste[i + 1];
			liste[i + 1] = tmp;
			i--;
		}
	}
}

void plus_proche_voisin_randomised(probleme& p, solution& s)
{
	s.cout = 0;
	int i;

	int nb_ville_proche = 0;
	ville_dist list_proche[taille_liste_plus_proche];

	int ville_rest[nMaxClient];
	// On exclus le dépot qui est compris dans le nb_ville
	int nb_ville_rest = p.nb_ville - 1;

	for (i = 1; i < p.nb_ville; i++)
	{
		ville_rest[i - 1] = i;
	}

	s.itineraire[0] = p.depot;
	for (i = 1; i < p.nb_ville; i++)
	{
		for (int k = 0; k < taille_liste_plus_proche; k++)
		{
			list_proche[k].dist = 99999999;
			list_proche[k].ville = -1;
			list_proche[k].idx_rest = -1;
		}
		nb_ville_proche = 0;

		for (int j = 0; j < nb_ville_rest; j++)
		{
			int dist = p.dist[s.itineraire[i - 1]][ville_rest[j]];
			if (dist != 0)
			{
				ville_dist current;
				current.dist = dist;
				current.ville = ville_rest[j];
				current.idx_rest = j;
				inserer_trie(list_proche, current);
				nb_ville_proche++;
				if (nb_ville_proche > taille_liste_plus_proche) { nb_ville_proche--; }
			}
		}

		int j = 0;
		while (j < nb_ville_proche - 1 && rand() % 101 > 80)
		{
			j++;
		}
		s.itineraire[i] = list_proche[j].ville;
		s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];

		//cout << s.itineraire[i - 1] << ' ' << s.itineraire[i] << ':' << p.dist[s.itineraire[i]][s.itineraire[i - 1]] << '\n';

		nb_ville_rest--;
		ville_rest[list_proche[j].idx_rest] = ville_rest[nb_ville_rest];
	}
	s.itineraire[i] = p.depot;
	s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];
}

void mega_heuristique_de_la_mort_qui_tue(probleme& p, solution& s)
{
	s.cout = 0;
	int i;

	int ville_rest[nMaxClient];
	// On exclus le dépot qui est compris dans le nb_ville
	int nb_ville_rest = p.nb_ville - 1;

	for (i = 1; i < p.nb_ville; i++)
	{
		ville_rest[i - 1] = i;
	}

	s.itineraire[0] = p.depot;
	for (i = 1; i < p.nb_ville; i++)
	{
		float rng = rand_float(0.01f, 0.5);

		float dist_min = 1e20f;
		int idx_ville_min = 0;
		for (int j = 0; j < nb_ville_rest; j++)
		{
			float d1 = p.dist[s.itineraire[i - 1]][ville_rest[j]];
			float eloignement_depot = (float)(p.dist[p.depot][ville_rest[j]] - p.dist[s.itineraire[i - 1]][p.depot]);
			float dist = d1 - rng * eloignement_depot;

			if (dist_min > dist)
			{
				dist_min = dist;
				idx_ville_min = j;
			}
		}
		s.itineraire[i] = ville_rest[idx_ville_min];

		nb_ville_rest--;
		ville_rest[idx_ville_min] = ville_rest[nb_ville_rest];
		s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];

	}
	s.itineraire[i] = p.depot;
	s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];
}

void meilleur_heuristique(probleme& p, solution& s, void (*heuritique)(probleme& p, solution& s), int nbTentative = 2000)
{
	solution meilleur = s;
	meilleur.cout = 999999999;
	repeat(i, nbTentative)
	{
		heuritique(p, s);
		if (s.cout < meilleur.cout)
		{
			meilleur = s;
		}
	}
	s = meilleur;
}
#pragma endregion


void afficher_itineraire(probleme& p, solution& s)
{
	cout << s.itineraire[0];
	for (int i = 1; i <= p.nb_ville; i++)
	{
		cout << " -> " << s.itineraire[i];
	}
	cout << "\n";
}



bool appliquer_2OPT(probleme& p, solution& s)
{
	int delta;
	int i;
	int j;
	bool trouver_meilleur = false;

	for (i = 1; i < p.nb_ville; i++)
	{
		for (j = i+2; j < p.nb_ville; j++)
		{
			delta =   p.dist[ s.itineraire[i]   ][ s.itineraire[j]   ]
					+ p.dist[ s.itineraire[i+1] ][ s.itineraire[j+1] ] 
					- p.dist[ s.itineraire[i]   ][ s.itineraire[i+1] ] 
					- p.dist[ s.itineraire[j]   ][ s.itineraire[j+1] ] ;
			if (delta < 0)
			{
				trouver_meilleur = true;
				while (i < j)
				{
					int tmp = s.itineraire[i];
					s.itineraire[i] = s.itineraire[j];
					s.itineraire[j] = tmp;
					i++; j--;
				}
			}
		}
	}
	return trouver_meilleur;
}


/// <summary>
/// Décale les éléments de l'itinéraire vers la droite en ramenant le dernier indice au début.
/// Bornes dep et fin non incluses
/// </summary>
/// <param name="s">solution contenant l'itinéraire</param>
/// <param name="dep">fin < dep</param>
/// <param name="fin">fin < dep</param>
void decal_gauche(solution& s, int dep, int fin)
{
	int cur = fin + 1;
	int buffer = s.itineraire[cur];
	while (cur + 1 < dep)
	{
		s.itineraire[cur] = s.itineraire[cur + 1];
		cur++;
	}
	s.itineraire[cur] = buffer;
}

/// <summary>
/// Décale les éléments de l'itinéraire vers la gauche en ramenant le dernier indice au début.
/// Bornes dep et fin non incluses
/// </summary>
/// <param name="s">solution contenant l'itinéraire</param>
/// <param name="dep">dep < fin</param>
/// <param name="fin">dep < fin</param>
void decal_droite(solution& s, int dep, int fin)
{
	int cur = fin - 1;
	int buffer = s.itineraire[cur];
	while (cur - 1 > dep)
	{
		s.itineraire[cur] = s.itineraire[cur - 1];
		cur--;
	}
	s.itineraire[cur] = buffer;
}
void appliquer_Insertion(probleme& p, solution& s)
{
	int delta;
	for (int i = 0; i < p.nb_ville - 1; i++)
	{
		for (int j = 0; j < p.nb_ville - 1; j++)
		{
			if (i != j && i-1 != j)
			{
				
				delta =   p.dist[ s.itineraire[i]   ][ s.itineraire[j]   ]
					    + p.dist[ s.itineraire[i-1] ][ s.itineraire[i+1] ] 
					    + p.dist[ s.itineraire[i]   ][ s.itineraire[j+1] ] 
					    - p.dist[ s.itineraire[i]   ][ s.itineraire[i-1] ] 
					    - p.dist[ s.itineraire[i]   ][ s.itineraire[i+1] ] 
					    - p.dist[ s.itineraire[j]   ][ s.itineraire[j+1] ] ;
				if (delta < 0)
				{
					if (i < j)
					{
						decal_gauche(s, j, i);
					}
					else
					{
						decal_droite(s, j, i);
					}
				}
			}
		}
	}
}

void afficher_cout(solution& s)
{
	cout << "> " << s.cout / 1000.0 << " km\n";
}

void split(probleme& p, solution& s)
{
	unused(p);
	unused(s);
	/*
	float m[nMaxClient] = { 1e20 };
	int pere[nMaxClient] = { 0 };
	m[0] = 0;

	int n = p.nb_ville;
	int j = 0;

	int capacityMax = 1200;

	int vol;*/

	/*
	repeat(i, n-1)
	{
		j = i + 1;

		cout =

		do
		{

		} while ((j <= n) && (vol + p.);
	}*/
}

void init_solution_par_defaut(probleme& p, solution& s)
{
	solution s_randomized;
	solution s_plus_proche;
	solution s_mega_heuristique;

	// Déterministe, l'appeler 1 fois ou plusieurs fois donne le même résultat
	// meilleur_heuristique(p, s_plus_proche, plus_proche_voisin);
	plus_proche_voisin(p, s_plus_proche);

	meilleur_heuristique(p, s_randomized, plus_proche_voisin_randomised);
	meilleur_heuristique(p, s_mega_heuristique, mega_heuristique_de_la_mort_qui_tue);

	s = (s_randomized.cout < s_plus_proche.cout) ?
		(s_randomized.cout < s_mega_heuristique.cout ? s_randomized : s_mega_heuristique)
		: (s_plus_proche.cout < s_mega_heuristique.cout ? s_plus_proche : s_mega_heuristique);

	cout << "Probleme de la ville de " << p.nom << "\n";

	cout << "   heuristique par defaut : \n";

	cout << "      > plus proche " << s_plus_proche     .metres / 1000.0 << " km\n";
	cout << "      > randomized  " << s_randomized      .metres / 1000.0 << " km\n";
	cout << "      > eloignement " << s_mega_heuristique.metres / 1000.0 << " km\n";
	//afficher_itineraire(p, s_plus_proche);
	//afficher_itineraire(p, s_randomized);
	//afficher_itineraire(p, s_mega_heuristique);
}


void resoudre_probleme(probleme& p)
{
	solution s;
	init_solution_par_defaut(p, s);

	repeat(i, 10)
	{
		appliquer_2OPT(p, s);
		cout << "" << s.metres / 1000.0 << " km\n";
	}

	cout << "\n";
}

int main()
{

	probleme problemes[] =
	{
		{
			#include "Paris.txt"
		},
		{
			#include "Puy2Dome.txt"
		},
		{
			#include "Allier.txt"
		},
	};

	int problemes_size = sizeof(problemes) / sizeof(problemes[0]);

	repeat(i, problemes_size)
	{
		resoudre_probleme(problemes[i]);
	}

	return EXIT_SUCCESS;
}


#pragma warning(pop)