<!DOCTYPE TS><TS>
<context>
    <name>ControlWindow</name>
    <message>
        <source>Folds recognition</source>
        <translation>Reconnaissance des sillons corticaux</translation>
    </message>
    <message>
        <source>Cortical folds recognition package</source>
        <translation>Module de reconaissance automatique des sillons corticaux</translation>
    </message>
</context>
<context>
    <name>QAnnealParams</name>
    <message>
        <source>Annealing</source>
        <translation>Recuit simulé</translation>
    </message>
    <message>
        <source>Load config</source>
        <translation>Charger une configuration</translation>
    </message>
    <message>
        <source>Save config</source>
        <translation>Sauver la configuration</translation>
    </message>
    <message>
        <source>Threaded interface</source>
        <translation>Interface &apos;threadée&apos; (calcul en parallèle de l&apos;interface graphique)</translation>
    </message>
    <message>
        <source>Using threads allows to keep Anatomist user
interface active during the several-hours
long annealing process, and allows to stop it
but it&apos;s more subject to bugs, so don&apos;t play
too much with Anatomist during the process,
it can easily crash.
If not threaded, you won&apos;t be able to use
this Anatomist until the relaxation is 
finished (hope 2-4 hours...)
(expect ~400 passes for a complete recognition)</source>
        <translation>L&apos;utilisation de threads permet de garder l&apos;interface
utilisateur d&apos;Anatomist active pendant le processus de recuit
qui dure plusieurs heures, et permet de le stopper,
mais ce mode est plus sujet à des bugs, alors ne jouez pas
trop avec Anatomist durant le processus, ça peut planter.
En mode non-threadé, vous ne pourrez pas utiliser cet
Anatomist avant que la relaxation soit finie
(comptez 2-4 heures...)
(attendez-vous à ~400 passes pour une reconnaissance complète)</translation>
    </message>
    <message>
        <source>Initialize annealing :</source>
        <translation>Initialiser le recuit :</translation>
    </message>
    <message>
        <source>If not set, annealing will not be initialized
(labels will be left unchanged, to continue an interrupted relaxation for instance)</source>
        <translation>Si mis à &apos;non&apos;, le recuit ne sera pas initialisé
(les étiquettes seront laissées dans leur état actuel,
pour continuer une relaxation interrompue par ex.)</translation>
    </message>
    <message>
        <source>yes</source>
        <translation>oui</translation>
    </message>
    <message>
        <source>no</source>
        <translation>non</translation>
    </message>
    <message>
        <source>Mode :</source>
        <translation>Mode :</translation>
    </message>
    <message>
        <source>Annealing transition accept/reject mode :
Gibbs sampler, Metropolis or ICM (deterministic)</source>
        <translation>Mode des transitions du recuit :
Échantillonneur de Gibbs, Metropoplis ou ICM (déterministe)</translation>
    </message>
    <message>
        <source>Iteration mode :</source>
        <translation>Mode d&apos;itération :</translation>
    </message>
    <message>
        <source>Transitions iterate mode
don&apos;t select CLIQUE - it&apos;s useless !</source>
        <translation>Mode d&apos;itération sur les transitions
Ne sélectionnez pas CLIQUE - ça ne sert à rien !</translation>
    </message>
    <message>
        <source>Translation file :</source>
        <translation>Fichier de traduction des étiquettes :</translation>
    </message>
    <message>
        <source>Labels translation file
Tells how to translate elements names
from one nomenclature to the model one.
If none, the default one is used
(see SiGraph library for details)</source>
        <translation>Fichier de traduction des étiquettes
Indique comment traduire les noms des éléments
d&apos;une nomenclature à celle du modèle.
S&apos;il n&apos;est pas précisé, la traduction par défaut est
utilisée (void la doc de SiGraph pour les détails)</translation>
    </message>
    <message>
        <source>&lt;default&gt;</source>
        <translation>&lt;défaut&gt;</translation>
    </message>
    <message>
        <source>Temperature :</source>
        <translation>Température :</translation>
    </message>
    <message>
        <source>Starting annealing temperature, will decrease during the process</source>
        <translation>Température de début du recuit, elle va
décroitre au cours du processus</translation>
    </message>
    <message>
        <source>Rate :</source>
        <translation>Taux :</translation>
    </message>
    <message>
        <source>Temperature decreasing rate - at each pass
the temperature will be multiplied by this factor</source>
        <translation>Taux de décroissance de la température - à chaque passe
la température est multipliée par ce facteur</translation>
    </message>
    <message>
        <source>ICM switching temp :</source>
        <translation>Temp. passage en ICM :</translation>
    </message>
    <message>
        <source>When the temperature gets lower than this
threshold, annealing automatically switches
to ICM deterministic mode.
If left to 0, ICM is used only after a whole pass
with no changes</source>
        <translation>Lorsque la température passe en dessous de ce seuil,
le recuit passe automatiquement en mode ICM déterministe.
Si laissé à 0, l&apos;ICM est utilisé après une passe entière sans changements</translation>
    </message>
    <message>
        <source>Stop rate :</source>
        <translation>Taux d&apos;arrêt :</translation>
    </message>
    <message>
        <source>% of accepted transitions below which the annealing stops</source>
        <translation>% de transitions acceptées en dessous duquel le recuit s&apos;arrête</translation>
    </message>
    <message>
        <source>Verbose :</source>
        <translation>Verbeux :</translation>
    </message>
    <message>
        <source>If set, verbose mode prints lots of counters
to keep you awaken during relaxation</source>
        <translation>Le mode verbeux affiche un tas de compteurs
pour vous tenir éveillé durant la relaxation</translation>
    </message>
    <message>
        <source>Gibbs nodes changes :</source>
        <translation>Noeuds changés (Gibbs) :</translation>
    </message>
    <message>
        <source>Number of nodes allowed to change simultaneously to form a Gibbs transition
LEAVE IT TO 1 FOR FOLDS RECOGNITION, or annealing will last for months...</source>
        <translation>Nombre de noeuds autorisés à changer simultanément
pour former une transition de Gibbs
LAISSEZ-LE A 1 POUR LA RECONNAISSANCE DES SILLONS,
ou bien le recuit durera des mois...</translation>
    </message>
    <message>
        <source>Remove brain :</source>
        <translation>Enlever cerveau :</translation>
    </message>
    <message>
        <source>Removes possible &apos;brain&apos; nodes in graph -
this shouldn&apos;t happen in newer graphs, but it&apos;s recommended to leave &apos;yes&apos;</source>
        <translation>Enlève les noeuds éventuels d&apos;étiquette
&apos;cerveau&apos; dans le graphe - il ne devrait pas y en
avoir dans les graphes récents, mais il est
recommandé de laisser &apos;oui&apos;</translation>
    </message>
    <message>
        <source>Set weights :</source>
        <translation>Fixer les poids :</translation>
    </message>
    <message>
        <source>Allows to set weights on each model element :
  0 : don&apos;t set anything (leaves it as in the model file)
 -1 : explicitly unsets the weights (RECOMMENDED)
t&gt;0 : sets nodes weights to t x num of relations</source>
        <translation>Permet de fixer les poids de chaque élément du modèle :
  0 : ne fixe rien (laisse les valeurs du fichier modèle)
 -1 : enlève explicitement tous les poids (RECOMMANDÉ)
t&gt;0 : fixe les poids des noeuds du modèle à t x nb de relations</translation>
    </message>
    <message>
        <source>Output plot file :</source>
        <translation>Fichier plot de sortie :</translation>
    </message>
    <message>
        <source>If a file is specified here, a &apos;plot file&apos; will
be written during relaxation, with a line for 
each pass, containing temperatures, numbers of
accepted transitions, energies, etc.
- Can be viewed with gnuplot for instance</source>
        <translation>Si un fichier est spécifié ici, un &apos;fichier de plot&apos; sera
écrit pendant la relaxation, avec une ligne pour chaque
passe, contenant les températures, les nombres de transitions
acceptées, les énergies, etc.
- peut être visualisé avec gnuplot par ex.</translation>
    </message>
    <message>
        <source>&lt;none&gt;</source>
        <translation>&lt;aucun&gt;</translation>
    </message>
    <message>
        <source>Initial labels :</source>
        <translation>Etiquettes initiales :</translation>
    </message>
    <message>
        <source>if VOID, all labels are initialized to the void (unrecognized) value (see below)
If NONE, labels are not initialized (left unchanged)
if RANDOM, labels are randomized: each node gets one of its possible labels</source>
        <translation>VOID : toutes les étiquettes sont initialisées à la valeur vide (non-reconnu, voir plus bas)
NONE : les étiquettes ne sont pas initialisées
RANDOM : les étiquettes sont initialisées aléatoirement,
 pour chaque noeud à une de ses étiquettes possibles</translation>
    </message>
    <message>
        <source>Void label :</source>
        <translation>Etiquette sans nom :</translation>
    </message>
    <message>
        <source>Label used for unrecognized nodes</source>
        <translation>Étiquette utilisée pour les noeuds non reconnus</translation>
    </message>
    <message>
        <source>Void pass mode :</source>
        <translation>Mode de passes &apos;void&apos; :</translation>
    </message>
    <message>
        <source>Void pass is a special relaxation pass wich can
occur from time to time to increase annealing performance :
it consists in trying to &apos;remove&apos; a whole fold
in a single transition to avoid aberrant labels distributions to persist

NONE : don&apos;t perform such special passes
REGULAR : perform them regularly, with occurency given below
STOCHASTIC : perform them irregularly on a mean probability
 based on the occurency below</source>
        <translation>Une passe Void est une passe spéciale de relaxation
qui peut être utilisée de temps en temps pour augmenter
les performances du recuit :
elle consiste à essayer d&apos; &apos;enlever&apos; un sillon entier
en une seule transition pour éviter de conserver des
distributions d&apos;étiquettes aberrantes

NONE : n&apos;effectue pas ces passes spéciales
REGULAR : effectue ces passes régulièrement, selon l&apos;occurence donnée après
STOCHASTIC : effectue ces passes de manière irrégulière, selon une
probabilité basée sur l&apos;occurence</translation>
    </message>
    <message>
        <source>Void pass occurency :</source>
        <translation>Occurence des passes &apos;void&apos;</translation>
    </message>
    <message>
        <source>Occurency (1 out of n) or mean inverse probability
of Void passes (if used)</source>
        <translation>Occurence (1 sur n) ou inverse de la probabilité
moyenne des passes Void (si elles sont utilisées)</translation>
    </message>
    <message>
        <source>Extension mode :</source>
        <translation>Mode d&apos;extension :</translation>
    </message>
    <message>
        <source>List of extended passes used during relaxation
Extended passes are plug-ins that can be inserted
in the annealing process.
Up to now 2 extensions exist :

CONNECT_VOID is similar to Void passes, but only
 tries to remove one connected component of
 a fold at e time
CONNECT inversely tries to mute a connected 
 component of void label nodes to the same fold label
 - useful after VOID and/or CONNECT_VOID  passes

Both CONNECT_VOID and CONNECT passes seem to
significantly improve the recognition, so you
should certainly use them</source>
        <translation>Liste de passe étendues utilisées au cours de la relaxation
Les passe étendues sont des `plug-ins&apos; qui peuvent être insérées
dans le processus du recuit.
A présent, 2 extensions existent :

CONNECT_VOID est similaire aux passes Void, mais essaie
 seulement d&apos;enlever une composante conexe de sillon
 à la fois
CONNECT, inversement, essaie de transformer une composante
 connexe de noeuds non étiquetés en un même sillon
 - utile après une passe VOID et/ou CONNECT_VOID

Les passes CONNECT_VOID et CONNECT semblent améliorer
significativement la reconnaissance, vous devriez donc
certainement les utiliser</translation>
    </message>
    <message>
        <source>Extension pass occurency :</source>
        <translation>Occurence des passes d&apos;extension :</translation>
    </message>
    <message>
        <source>Occurency (1 out of n) of extended passes
Up to now they happen regularly in their given
order; if occurency is the same as Void passes,
Void pass always happens first, then immediately
followed by extended passes</source>
        <translation>Occurence (1 sur n) des passes étendues.
Jusqu&apos;à maintenant elles surviennent régulièrement
dans leur ordre indiqué; si l&apos;occurence est la même
que celle des passes Void, les passes Void s&apos;activent
toujours en premier, et sont immédiatement suivies
des passes étendues</translation>
    </message>
    <message>
        <source>Double drawing lots :</source>
        <translation>Double tirage :</translation>
    </message>
    <message>
        <source>Performs 2 drawing lots before accepting a transition
(if my memory is good enough...)
This leads to accept only transitions with a high
probability, or no change at all.
NOT RECOMMENDED - it seems to give bad results
and it&apos;s theoretically an heresy...</source>
        <translation>Effectue 2 tirages au sort avant d&apos;accepter une transition
(si ma mémoire est bonne...)
Cela amène à n&apos;accepter que les transitions les plus
probables, ou aucun changement.
NON RECOMMANDÉ - cela semble produire de mauvais résultats
et du point de vue théorique, c&apos;est une hérésie...</translation>
    </message>
    <message>
        <source>Start relaxation</source>
        <translation>Commencer la relaxation</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>Arrêter</translation>
    </message>
    <message>
        <source>Annealing configuration</source>
        <translation>Configuration du recuit simulé</translation>
    </message>
    <message>
        <source>Open annealing configuration</source>
        <translation>Charger une configuration de recuit</translation>
    </message>
    <message>
        <source>Translation file</source>
        <translation>Fichier de traduction :</translation>
    </message>
    <message>
        <source>Open translation file</source>
        <translation>Charger un fichier de traduction</translation>
    </message>
    <message>
        <source>Output plot file</source>
        <translation>Fichier plot de sortie :</translation>
    </message>
    <message>
        <source>Select output plot file</source>
        <translation>Fichier plot de sortie</translation>
    </message>
    <message>
        <source>Save annealing config file</source>
        <translation>Sauvegarde de la config de recuit simulé</translation>
    </message>
</context>
<context>
    <name>QFFoldCtrl</name>
    <message>
        <source>Fold Fusion control</source>
        <translation>Contrôle de l&apos;interaction sillons - modèle</translation>
    </message>
    <message>
        <source>Node mapping mode</source>
        <translation>Mode de représentation des noeuds</translation>
    </message>
    <message>
        <source>Node potentials</source>
        <translation>Potentiels des noeuds</translation>
    </message>
    <message>
        <source>Total potentials</source>
        <translation>Potentiels totaux</translation>
    </message>
    <message>
        <source>Labels</source>
        <translation>Étiquettes</translation>
    </message>
    <message>
        <source>Weights only (nodes)</source>
        <translation>Poids seuls (noeuds)</translation>
    </message>
    <message>
        <source>Weights only (total)</source>
        <translation>Poids seuls (totaux)</translation>
    </message>
    <message>
        <source>Edge mapping</source>
        <translation>Représentation des relations</translation>
    </message>
    <message>
        <source>Show edge potentials</source>
        <translation>Montrer les potentiels des relations</translation>
    </message>
    <message>
        <source>Modifiers</source>
        <translation>Modificateurs</translation>
    </message>
    <message>
        <source>Model</source>
        <translation>Modèle</translation>
    </message>
    <message>
        <source>Weighted potentials</source>
        <translation>Potentiels pondérés</translation>
    </message>
    <message>
        <source>0 potential at mid-colormap</source>
        <translation>Potentiel 0 au milieu de l&apos;échelle</translation>
    </message>
    <message>
        <source>0 potential with different color : </source>
        <translation>Potentiel 0 avec une couleur différente :</translation>
    </message>
    <message>
        <source>No potential color : </source>
        <translation>Couleur pour l&apos;absence de potentiel :</translation>
    </message>
    <message>
        <source>Contribution of edge numbers</source>
        <translation>Contribution du nombre de relations</translation>
    </message>
    <message>
        <source>Update model</source>
        <translation>Rafraîchir</translation>
    </message>
    <message>
        <source>Potential 0 color</source>
        <translation>Couleur pour le potentiel 0</translation>
    </message>
    <message>
        <source>Color for &apos;no potential&apos;</source>
        <translation>Couleur pour &apos;pas de potentiel&apos;</translation>
    </message>
</context>
<context>
    <name>QSelectMenu</name>
    <message>
        <source>Color</source>
        <translation>Couleur</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation>Palette</translation>
    </message>
    <message>
        <source>Material</source>
        <translation>Matériau</translation>
    </message>
    <message>
        <source>Fusion</source>
        <translation>Fusion</translation>
    </message>
    <message>
        <source>Control graphs fusion</source>
        <translation>Contrôle de la fusion de graphes</translation>
    </message>
    <message>
        <source>Annealing</source>
        <translation>Recuit simulé</translation>
    </message>
</context>
</TS>
