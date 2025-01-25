
/**
 * Message contenant des informations ou des demandes sur l'air d'une
 * pi�ce. S'il est de type "mesure", il contient alors la valeur de la
 * temp�rature courante de l'air. S'il est de type "chauffer", il
 * contient une demande de chauffage � effectuer.
 */
public class MessageTemperature implements java.io.Serializable {

    /**
     * Constante pr�cisant que le message est de type "mesure"
     * (contient la temp�rature courante de l'air)
     */
    public final static byte MESURE = 0;

    /**
     * Constante pr�cisant que le message est de type "chauffer"
     * (contient une demnande de chauffage de l'air) 
     */
    public final static byte CHAUFFER = 1;

    /**
     * Nom de la pi�ce
     */ 
    protected String piece;

    /**
     * Pour un message de type "messure", contient la temp�rature
     * courante de l'air. Pour un message de type "chauffer", contient
     * la puissance du chauffage demand�e (entre 0 et 5).
     */
    protected int valeur;
    
    /**
     * Type du message, pr�cis� par une des 2 constantes
     * <code>MESURE</code> ou <code>CHAUFFER</code>
     */
    protected byte type;

    /**
     * Retourne la valeur stock�e dans le message.
     */
    public int getValeur()
    {
	return valeur;
    }

    /**
     * Retourne le type du message (<code>MESURE</code> ou
     * <code>CHAUFFER</code>) 
     */
    public int getType()
    {
	return type;
    }
    
    /**
     * Retourne le nom de la pi�ce
     */
    public String getPiece()
    { 
	return piece;
    }

    /**
     * Convertit le message en son �quivalent en tableau de byte.
     */
    public byte[] toBytes()
    {
        byte[] tab = new byte[piece.length() + 5];

        int val = valeur;

        for (int i=0; i < 4; i++)
	    {
            tab[i] = (byte) (val & 0x000000FF);
            val = val >>> 8;
        }

        tab[4] = type;

        byte[] tabPiece = piece.getBytes();

        System.arraycopy(tabPiece, 0, tab, 5, piece.length());

        return tab;
    }
    /**
     * Retourne un message � partir de son �quivalent en tableau de byte.
     * @param tab le tableau de byte contenant le message
     * @param length le nombre de cases � consid�rer dans le tableau
     * @return une instance de message initialis�e avec le contenu du
     * tableau 
     */
    public static MessageTemperature fromBytes(byte[] tab, int length)
    {
	    int[] val = new int[4];
      
	    for (int i=0; i < 4; i++)
	    {
            if (tab[i] < 0)
                val[i] = (tab[i] + 256) << (i * 8);
            else
                val[i] = tab[i] << (i * 8);
	    }
        int valeur = val[0] | val[1] | val[2] | val[3];

        String piece = new String(tab, 5, length - 5);

        return new MessageTemperature(valeur, tab[4], piece);
    }

    public String toString() 
    {
        String msg = "type = ";
        if (type == MessageTemperature.MESURE) msg += "mesure ";
        else if (type == MessageTemperature.CHAUFFER) msg+= "modifier ";
        else msg += "inconnu ";
        msg += ", valeur = " + valeur + ", piece = " + piece;
        return msg;
    }
    
    /**
     * Cr�e un nouveau message.
     * @param valeur le niveau de temp�rature ou la puissance du chauffage
     * @param type le type du message (<code>MESURE</code> ou <code>CHAUFFER</code>)
     * @param piece le nom de le pi�ce consid�r�e
     */
    public MessageTemperature(int valeur, byte type, String piece)  
    {
        this.valeur = valeur;
        this.type = type;
        this.piece = piece;
    }
}