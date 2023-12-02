#include <iostream>
#include <fstream>
#include <string>

#define repeat(_varName, _nbTime) for(int _varName = 0, nb_time = (int)(_nbTime); _varName < nb_time; _varName++)
#pragma warning(push)
// Faux positif 
#pragma warning(disable:6385)
// Faux positif sur la lecture hors tableau � cause de la nature du programme avec plein de tableaux dont les indices sont lu depuis d'autre tableaux
#pragma warning(disable:6386)
// La taille de certaine structure est trop grande, mais on ne veut pas d'allocations dynamique
#pragma warning(disable:6262)

using namespace std;

const int nMaxClient = 100;

typedef struct probleme
{
	// Indice de ville du d�pot (souvent 0)
	int depot;
	// Nombre de ville, d�pot compris
	int nb_ville;
	int dist[nMaxClient][nMaxClient];
	int qte[nMaxClient];
	int capacite;
} probleme;


typedef struct solution
{
	int itineraire[nMaxClient + 2];
	int m[nMaxClient];
	int pere[nMaxClient];
	int cout;
} solution;

void lire_fichier(probleme& p, string fileName)
{
	string word;
	ifstream file(fileName.c_str());

	file >> p.depot;
	file >> p.nb_ville;
	file >> p.capacite;

	for (int i = 0; i < p.nb_ville; i++)
	{
		for (int j = 0; j < p.nb_ville; j++)
		{
			file >> p.dist[i][j];
		}
	}

	string line;
	getline(file, line); // le commentaire

	repeat(i, p.nb_ville)
	{
		int useless;
		file >> useless;
		file >> p.qte[i];
	}

	return;
}

int rand_uni(int min, int max)
{
	return min + rand() % (max - min + 1);
}

void plus_proche_voisin(probleme& p, solution& s)
{
	int i;
	int position = p.depot;

	int vill_rest[nMaxClient];
	// On exclus le d�pot qui est compris dans le nb_ville
	int nb_vill_rest = p.nb_ville - 1;

	for (i = 1; i < p.nb_ville; i++)
	{
		vill_rest[i - 1] = i;
	}

	s.itineraire[0] = p.depot;
	for (i = 1; i < p.nb_ville; i++)
	{
		int dist_min = 999999;
		int idx_ville_min = 0;
		for (int j = 0; j < nb_vill_rest; j++)
		{
			int dist = p.dist[s.itineraire[i - 1]][vill_rest[j]];
			if (dist_min > dist && dist != 0)
			{
				dist_min = dist;
				idx_ville_min = j;
			}
		}
		s.itineraire[i] = vill_rest[idx_ville_min];

		nb_vill_rest--;
		vill_rest[idx_ville_min] = vill_rest[nb_vill_rest];
	}
	s.itineraire[i] = p.depot;
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
	int position = p.depot;

	int nb_ville_proche = 0;
	ville_dist list_proche[taille_liste_plus_proche];

	int vill_rest[nMaxClient];
	// On exclus le d�pot qui est compris dans le nb_ville
	int nb_vill_rest = p.nb_ville - 1;

	for (i = 1; i < p.nb_ville; i++)
	{
		vill_rest[i - 1] = i;
	}

	s.itineraire[0] = p.depot;
	for (i = 1; i < p.nb_ville; i++)
	{
		for (int k = 0; k < taille_liste_plus_proche; k++)
		{
			list_proche[k].dist = 99999;
			list_proche[k].ville = -1;
			list_proche[k].idx_rest = -1;
		}
		nb_ville_proche = 0;

		for (int j = 0; j < nb_vill_rest; j++)
		{
			int dist = p.dist[s.itineraire[i - 1]][vill_rest[j]];
			if (dist != 0)
			{
				ville_dist current;
				current.dist = dist;
				current.ville = vill_rest[j];
				current.idx_rest = j;
				inserer_trie(list_proche, current);
				nb_ville_proche++;
				if (nb_ville_proche > taille_liste_plus_proche) { nb_ville_proche--; }
			}
		}

		int j = 0;
		while (j < nb_ville_proche - 1 && rand() % 10 >= 8)
		{
			j++;
		}
		s.itineraire[i] = list_proche[j].ville;
		s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];
		//cout << s.itineraire[i - 1] << ' ' << s.itineraire[i] << ':' << p.dist[s.itineraire[i]][s.itineraire[i - 1]] << '\n';

		nb_vill_rest--;
		vill_rest[list_proche[j].idx_rest] = vill_rest[nb_vill_rest];
	}
	s.itineraire[i] = p.depot;
	s.cout += p.dist[s.itineraire[i]][s.itineraire[i - 1]];

}


void afficher_itineraire(probleme& p, solution& s)
{
	cout << s.itineraire[0];
	for (int i = 1; i <= p.nb_ville; i++)
	{
		cout << " -> " << s.itineraire[i];
	}
	cout << "\n";
}



void appliquer_2OPT(probleme& p, solution& s)
{
	int delta;
	int i;
	int j;

	for (i = 1; i < p.nb_ville; i++)
	{
		for (j = i + 2; j < p.nb_ville; j++)
		{
			delta = p.dist[s.itineraire[i]][s.itineraire[j]]
				+ p.dist[s.itineraire[i + 1]][s.itineraire[j + 1]]
				- p.dist[s.itineraire[i]][s.itineraire[i + 1]]
				- p.dist[s.itineraire[j]][s.itineraire[j + 1]];
			if (delta < 0)
			{
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
}


/// <summary>
/// D�cale les �l�ments de l'itin�raire vers la droite en ramenant le dernier indice au d�but.
/// Bornes dep et fin non incluses
/// </summary>
/// <param name="s">solution contenant l'itin�raire</param>
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
/// D�cale les �l�ments de l'itin�raire vers la gauche en ramenant le dernier indice au d�but.
/// Bornes dep et fin non incluses
/// </summary>
/// <param name="s">solution contenant l'itin�raire</param>
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
			if (i != j && i - 1 != j)
			{

				delta = p.dist[s.itineraire[i]][s.itineraire[j]]
					+ p.dist[s.itineraire[i - 1]][s.itineraire[i + 1]]
					+ p.dist[s.itineraire[i]][s.itineraire[j + 1]]
					- p.dist[s.itineraire[i]][s.itineraire[i - 1]]
					- p.dist[s.itineraire[i]][s.itineraire[i + 1]]
					- p.dist[s.itineraire[j]][s.itineraire[j + 1]];
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

void appliquer_split(probleme& p, solution& s)
{
	//Init

	for (int i = 0; i < p.nb_ville; i++)
	{
		s.m[i] = INT_MAX;
		s.pere[i] = 0;
	}
	s.m[0] = 0;
	

	// Split
	for (int i = 0; i < p.nb_ville; i++)
	{
		int val, cost;
		int j = i + 1;
		do
		{
			if (j == i+1)
			{
				cost = p.dist[p.depot][s.itineraire[j]] + p.dist[s.itineraire[j]][p.depot];
				val = p.qte[s.itineraire[j]];
				
				int current_cost = s.m[i] + cost;
				if (current_cost < s.m[j])
				{
					s.m[j] = current_cost;
					s.pere[j] = i;
				}
			}
			else
			{
				cost -= p.dist[s.itineraire[j-1]][p.depot] 		   + 
						p.dist[s.itineraire[j-1]][s.itineraire[j]] + 
						p.dist[s.itineraire[j  ]][p.depot]			;
				val  += p.qte [s.itineraire[j  ]];

				int current_cost = s.m[i] + cost;
				if (current_cost < s.m[j])
				{
					s.m[j] = current_cost;
					s.pere[j] = i;
				}
			}

		} while ((j < p.nb_ville) && (val + p.qte[s.itineraire[j]] <= p.capacite ));
		
	}
	
}

int main()
{
	probleme p;
	solution s;

	lire_fichier(p, "Paris.txt");

	//plus_proche_voisin(p, s);
	for (int i = 0; i < 1; i++)
	{
		plus_proche_voisin_randomised(p, s);
		afficher_itineraire(p, s);
		afficher_cout(s);

		appliquer_2OPT(p, s);
		afficher_itineraire(p, s);
		afficher_cout(s);
	}
	return 0;
}


#pragma warning(pop)