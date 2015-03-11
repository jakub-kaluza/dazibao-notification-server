###Compilation
Lancez simplement l'outil 'make'

###Lecture

La lecture se fait via le programme `reader`. 

`reader <options> file` 

options :  
* -h : convertit et affiche en un fichier HTML au lieu de données brutes. A utiliser avec une redirection de sortie (`reader -h fichier.dzb > fichier.html`) 


###Ecriture

L'ajout de contenu se fait via le programme `writer`. 

`writer <options> file [expressions]` 

Par défaut, on va parcourir l'expression et, si elle est valide (voir *format*), on l'ajoute à la fin du fichier. Ensuite on passe aux éventuelles expressions suivantes (séparées par des espaces).
options : 
* -d : pour ajouter automatiquement la date courante aux données ajoutées.
* -h : afficher l'aide

expression :  
Le format est le suivant : 
`<type>{<contenu>}`

où type est choisi parmi les lettres suivantes : 
* t : TEXT {texte brut}
* p : PNG (nom du fichier)
* j : JPEG (nom du fichier)
* m : MP3 (nom du fichier)
* o : OGG (nom du fichier)
* v : OGV (nom du fichier)
* w : WEBM (nom du fichier)
* 1 : Pad1 (1 octet vide)
* n : PadN (n octets vides)
* d : dated (date-contenu) *où date est soit un entier UTC, soit calculée automatiquement (si autre chose de non vide est spécifié) *
* c : compound (c1-c2-c3)

Exemple : 
Pour créer un Texte suivi d'un Compound composé d'un Texte, d'une image Jpeg Datée, et d'un Compound lui-même fait d'un Pad1 et d'un Texte  : 

`t{"BZH"}-c{t{"ma famille"}-d{32713-j{familleEnBretagne.jpeg}}-c{1-t{"tant de bonheur"}}}`

*notes : les guillemets ne sont pas obligatoires, mais permettent d'échapper les caractères d'espacement. Les espaces hors guillemets sont interdits.  *  
*Prendre garde aux caractères spéciaux du terminal : ',', '!', ... *



Exemple: ./writer exemple2.dzb 'm{hello.mp3}'



###Suppression
La suppression se fait avec le programme `deleter`. 

`deleter <nom de fichier>` 

actions possibles :  
-exit : quitter le programme
-del : supprimer la TLV courante
-n: passer à la TLV suivante
-e: explorer la TLV courante (dans le cas des compound et dated non vides)
-jump <nombre>  (où « nombre »est un nombre entier): faire un saut de <nombre> TLVs, à une profondeur donnée (par exemple si on est dans un compound, « jump 2 » va nous décaler de 2 TLV à l'intérieur de ce compound. Si le nombre dépasse le nombre de TLV : fin du programme (si on n'était pas à l'intérieur d'un type composé) ou on passe à la TLV suivante chez la TLV parent (dans le cas des types composés)


###Compaction

Pour compacter un fichier, il suffit de lancer le programme compacter.

`compacter <nom de fichier>` 



###Notifications

Lancer le programme serveur`dzbNotify`. 

`server <nom de fichier contenant des chemins de fichiers dazibao>` 

Vous pouvez maintenant lancer les programmes clients.
