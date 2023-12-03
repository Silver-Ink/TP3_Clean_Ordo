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
	int capacite;
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
	int mini[nMaxClient];
	int pere[nMaxClient];
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

void mauvaise_heuristique_rng(probleme& p, solution& s)
{
	s.cout = 0;
	int i;

	s.itineraire[0] = p.depot;
	for (i = 1; i < p.nb_ville; i++)
	{
		s.itineraire[i] = i;
		s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];
	}
	s.itineraire[i] = p.depot;
	s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];
}

void meilleur_heuristique(probleme& p, solution& s, void (*heuritique)(probleme& p, solution& s), int nbTentative = 1000)
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

const char* funny_string[] =
{
	", puis aller a ",
	", ensuite rendez vous a ",
	", continuer vers ",
	", puis ",
	", allez sur ",
	", se diriger vers ",
	", aller a ",
	", se rendre a ",
	", direction ",
	", continuer tout droit jusqu'a ",
	", depeche toi d'aller a ",
	", arreter-vous a ",
	". La ! Stop ! "
};

void afficher_itineraire(probleme& p, solution& s)
{
	cout << "\nitineraire :\n";
	cout << "Commencer au depot ";
	cout << s.itineraire[0];
	for (int i = 1; i <= p.nb_ville; i++)
	{
		//cout << " -> " << s.itineraire[i];
		cout << funny_string[rand()%(sizeof(funny_string)/sizeof(const char*))] << s.itineraire[i];
	}
	cout << "\n";
}
/// @brief Affiche les tournée dans l'ordre inverse, mais leur contenu est dans l'ordre
void afficher_tournee(probleme& p, solution& s)
{
	int id_deb = p.nb_ville - 1;
	int id_pos;
	int id_tourne = 0;

	while (id_deb != 0)
	{
		cout << "Tournee n." << id_tourne << " = [ " << p.depot << ", ";
		id_pos = id_deb;
		id_deb = s.pere[id_pos];
		
		for (int i = id_deb + 1; i <= id_pos; i++)
		{
			printf_s("%3d,", s.itineraire[i]);
		}

		cout << "  " << p.depot << " ]\n";
		id_tourne++;
	}
}



bool appliquer_2OPT(probleme& p, solution& s)
{
	bool trouver_meilleur = false;

	for (int i = 1; i < p.nb_ville - 1; i++)
	{
		for (int j = i + 2; j < p.nb_ville; j++)
		{
			int delta = p.dist[s.itineraire[i]][s.itineraire[j]] +
				p.dist[s.itineraire[i + 1]][s.itineraire[j + 1]] -
				p.dist[s.itineraire[i]][s.itineraire[i + 1]] -
				p.dist[s.itineraire[j]][s.itineraire[j + 1]];

			if (delta < 0)
			{
				trouver_meilleur = true;

				// Inverser le segment entre i et j
				for (int k = 0; k < (j - i) / 2; k++)
				{
					int tmp = s.itineraire[i + 1 + k];
					s.itineraire[i + 1 + k] = s.itineraire[j - k];
					s.itineraire[j - k] = tmp;
				}

				s.cout += delta;
			}
		}
	}
	return trouver_meilleur;
}



/// <summary>
/// Décale les éléments de l'itinéraire vers la gauche en ramenant le dernier indice au début.
/// Bornes debut et fin non incluse
/// </summary>
/// <param name="s">solution contenant l'itinéraire</param>
/// <param name="dep">fin < dep</param>
/// <param name="fin">fin < dep</param>
void decal_gauche(solution& s, int debut, int fin)
{
	int cur = debut;
	int buffer = s.itineraire[debut];
	while (cur + 1 <= fin)
	{
		s.itineraire[cur] = s.itineraire[cur + 1];
		cur++;
	}
	s.itineraire[cur] = buffer;
}

/// <summary>
/// Décale les éléments de l'itinéraire vers la droite en ramenant le dernier indice au début.
/// Bornes debut et fin non incluse
/// </summary>
/// <param name="s">solution contenant l'itinéraire</param>
/// <param name="dep">dep < fin</param>
/// <param name="fin">dep < fin</param>
void decal_droite(solution& s, int debut, int fin)
{
	int cur = fin;
	int buffer = s.itineraire[fin];
	while (cur - 1 >= debut)
	{
		s.itineraire[cur] = s.itineraire[cur - 1];
		cur--;
	}
	s.itineraire[cur] = buffer;
}


int appliquer_insertion(probleme& p, solution& s)
{
#define d(x,y) p.dist[s.itineraire[x]][s.itineraire[y]]

	int nbInsert = 0;
	int delta;
	for(int i = 1; i < p.nb_ville - 1; i++)
	{
		int delta_moins = 
			- d(i - 1, i)
			- d(i, i + 1)
			+ d(i - 1, i + 1);

		for (int j = 1; j < p.nb_ville - 1; j++)
		{
			if (i == j || i == j + 1) { continue; }

			int delta_plus = 
				- d(j, j + 1)
				+ d(j, i)
				+ d(i, j+1);

			int delta = delta_moins + delta_plus;

			if (delta < 0)
			{
				s.cout += delta;


				if (i < j)
				{
					decal_gauche(s, i, j);
				}
				else
				{
					decal_droite(s, j+1, i);
				}
				nbInsert++;
				break;
			}
		}
	}
	return nbInsert;
#undef d
}

void afficher_cout(solution& s)
{
	cout << "> " << s.cout / 1000.0 << " km\n";
}

void init_split(probleme& p, solution& s)
{
	for (int i = 0; i < p.nb_ville; i++)
	{
		s.mini[i] = 99999999;
		s.pere[i] = 0;
	}
	s.mini[0] = 0;
}

void appliquer_split(probleme& p, solution& s)
{
	init_split(p, s);
	repeat(i, p.nb_ville)
	{
		int volume, cost;
		int j = i + 1;
		do
		{
			if (j == i+1)
			{
				//cost = p.dist[p.depot][s.itineraire[j]] + p.dist[s.itineraire[j]][p.depot];
				cost = p.dist[p.depot][s.itineraire[j]] * 2;
				volume = p.qte[s.itineraire[j]];
				
				int current_cost = s.mini[i] + cost;
				if (current_cost < s.mini[j])
				{
					s.mini[j] = current_cost;
					s.pere[j] = i;
				}
			}
			else
			{ 
				cost =  cost 									   -
						p.dist[s.itineraire[j-1]][p.depot] 		   + 
						p.dist[s.itineraire[j-1]][s.itineraire[j]] + 
						p.dist[s.itineraire[j  ]][p.depot]			;
				volume  += p.qte [s.itineraire[j  ]];

				int current_cost = s.mini[i] + cost;
				if (current_cost < s.mini[j])
				{
					s.mini[j] = current_cost;
					s.pere[j] = i;
				}
			}
			j++;

		} while ((j < p.nb_ville) && (volume + p.qte[s.itineraire[j]] <= p.capacite ));
	}
}


void init_solution_par_defaut(probleme& p, solution& s, bool prendre_mauvaise_soluce = false)
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

	cout << "================= Probleme de la ville de " << p.nom << " =======\n";

	cout << "heuristique par defaut : \n";

	cout << "   > plus proche " << s_plus_proche     .metres / 1000.0 << " km\n";
	cout << "   > randomized  " << s_randomized      .metres / 1000.0 << " km\n";
	cout << "   > eloignement " << s_mega_heuristique.metres / 1000.0 << " km\n";

	if(prendre_mauvaise_soluce)
	{
		solution s_mauvaise;
		mauvaise_heuristique_rng(p, s_mauvaise);
		cout << "   > mauvaise rng " << s_mauvaise.metres / 1000.0 << " km\n";
		s = s_mauvaise;
	}
}


void resoudre_probleme(probleme& p)
{
	solution s;
	init_solution_par_defaut(p, s);
	cout << "\nrecherche de la plus courte tournee...\n";

	bool continuer;
	do
	{
		int nbInsertion = 0;
		int nb2OP = 0;
		continuer = false;
		//appliquer_split(p, s);

		//repeat(j, 100)

		int curInsert;
		while (curInsert = appliquer_insertion(p, s))
		{
			nbInsertion += curInsert;
			continuer = true;
		}

		if(appliquer_2OPT(p, s))
		{
			continuer = true;
			nb2OP++;
		}

		cout << "> " << s.metres / 1000.0 << " km apres " << nbInsertion << " insertions et " << nb2OP << " 2OPT \n";

	} while (continuer);

	afficher_itineraire(p, s);

	appliquer_split(p, s);

	afficher_tournee(p, s);

	cout << "\n\n";
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

	cout << " C'est la fin, et les algo tiennent bien sur un timbre poste" << endl;

	return EXIT_SUCCESS;
}


#pragma warning(pop)