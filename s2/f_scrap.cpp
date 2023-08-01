#include <map>
#include <tuple>

/* V této úloze budeme programovat hru pro tři hráče, která se
 * odehrává na šestiúhelníkové síti, která má sama tvar pravidelného
 * šestiúhelníku o hraně délky 4 pole. Každé pole má pevně určenou
 * «cenu», která udává, kolik kol se figura na tomto poli zdrží. Na
 * pole s cenou 20 nebo vyšší není možné vstoupit. Ceny jsou určeny
 * hodnotou typu ‹std::map›, kde klíčem je dvojice celých čísel.
 * Souřadnice (0, 0) odpovídají středu plochy, první souřadnice
 * stoupá směrem na sever (a klesá na jih), druhá souřadnice stoupá
 * směrem na severovýchod. Není-li buňka v mapě přítomná, má
 * implicitní zdržení 1. */

using coordinates = std::tuple< int, int >;
using terrain = std::map< coordinates, int >;

/* Hrací plocha obsahuje 6 speciálních polí, každé v jednom
 * „vrcholu“ hrací plochy. Počínaje severním cípem a pokračuje ve
 * směru hodinových ručiček to jsou:
 *
 *  1. startovní pole hráče 1 – souřadnice (3, 0),
 *  2. studna života – souřadnice (0, 3),
 *  3. startovní pole hráče 2 – souřadnice (-3, 3)
 *  4. studna energie – souřadnice (-3, 0),
 *  5. startovní pole hráče 3 – souřadnice (0, -3),
 *  6. zlatý důl – souřadnice (3, -3).
 *
 * Pole se zdroji (studny a důl) mají na začátku každé přiděleno 6
 * jednotek příslušného zdroje. */

/* Ve hře se objevují tři typy zdrojů: život, energie a zlato.
 * Celkový součet zdrojů ve hře musí být za všech okolností
 * zachován. Kdyby měl být nějaký zdroj zničen nebo je ho
 * z jakéhokoliv důvodu přebytek, který nelze na místě upotřebit, je
 * místo toho přesunut do své studny (dolu) na mapě. Zdroje jsou
 * libovolně dělitelné (reprezentujte je jako čísla typu ‹double›).
 * Zdroje lze sestrojit implicitně – takto sestrojený zdroj má
 * nulovou hodnotu.
 *
 * Typy ‹resource_X› nechť mají metody:
 *
 *  • ‹take› s parametrem ‹source› a volitelným parametrem ‹max›,
 *    která odebere ze zdroje ‹source› nejvýše ‹max› jednotek zdroje
 *    (ale ne víc, než jich je přítomno),
 *  • ‹value› – vrátí aktuální hodnotu zdroje. */

struct resource_life;
struct resource_mana;
struct resource_gold;

/* Na začátku hry dostane každý hráč 3 figury (kameny),
 * reprezentované hodnotami typu ‹piece›. Figury «nelze kopírovat»,
 * ani přiřazovat kopií, ale jinak s nimi může hráč libovolně
 * nakládat. */

/* Vstoupí-li figura na políčko se studnou (dolem), získá jednu
 * jednotku příslušného zdroje (není-li to možné, získá maximum,
 * které získat smí) a je vrácena na domovské pole svého vlastníka
 * (a otočena směrem do středu pole). V dalším kole hráč rozhodne
 * o jejím dalším cíli. */

struct piece
{
    /* Každá figura může vlastnit nějaké množství každého typu
     * zdroje. Figura ale za žádných okolností nemůže vlastnit víc
     * zlata než energie. */

    double life() const;
    double mana() const;
    double gold() const;

    /* Figuře lze přidat zdroje metodou ‹take› (zlato přidávat nelze
     * jinak, než vstupem na pole se zlatým dolem). Zdroje nelze
     * odebrat jinak, než zajetím nebo zničením. */

    void take( resource_life );
    void take( resource_mana );

    /* Hráč musí mít možnost zjistit, v jakém stavu se figura
     * nachází, aby se mohl rozhodovat co s ní provést. */

    bool in_conflict() const;
    bool need_destination() const;

    /* Je-li figura zničena, zdroje které vlastní, se přesunou podle
     * typu zdroje:
     *
     *  • polovina života je vrácena hráči, který figuru vlastnil,
     *    zbytek se vrátí do studny života,
     *  • třetina energie se rozdělí mezi protihráče rovným dílem,
     *    třetina se vrátí vlastníkovi a třetina se vrátí zpátky do
     *    studny energie,
     *  • veškeré zlato je předáno hráči, který figuru vlastnil (a
     *    zničil). */

    ~piece();
};

/* Hra jako celek je reprezentována typem ‹scrap› (popis níže). */

struct scrap;

/* Hráč je reprezentován hodnotou, která je typu odvozeného od třídy
 * ‹player›. Krom (nejvýše) tří figur může vlastnit volné zdroje (na
 * začátku hry 1 jednotku života, 1 jednotku energie a žádné zlato).
 * Volný život a energii může přidělit figuře, o které rozhoduje,
 * předtím než ji vloží do hry. Figura s nulovým životem nebo
 * energií do hry vstoupit nesmí – při pokusu o takový tah je
 * vyhozena výjimka typu ‹piece_dead›. */

struct player
{
    /* Metoda ‹play› se volá pouze v situaci, kdy může hráč nějaké
     * rozhodnutí učinit:
     *
     *  • na začátku hry musí rozhodnout o každé ze svých 3 figur,
     *  • dojde-li figura do cíle, je vrácena na domovské pole a je
     *    potřeba o ní opět rozhodnout,
     *  • rozhoduje se také v situaci, kdy se setkají figury různých
     *    hráčů na stejném poli.
     *
     * Figuru, o které je potřeba rozhodnout, dostane metoda ‹play›
     * jako parametr. Vybranou akci provede skrze parametr typu
     * ‹scrap› (definovaný níže).
     *
     * Metoda ‹play› nemusí s figurou provést žádnou akci (případně
     * ji může i zničit). Neprovede-li hráč akci, figura je mimo
     * hru. Hráč ji při libovolném volání ‹play› může do hry opět
     * vrátit. */

    virtual void play( scrap &, piece ) = 0;

    /* Příslušnou metodu ‹take› hra zavolá v situaci, kdy má hráči
     * předat nějaký zdroj. */

    virtual void take( resource_life ) = 0;
    virtual void take( resource_mana ) = 0;
    virtual void take( resource_gold ) = 0;

    /* Metoda ‹end› se zavolá na konci hry. Jedná se o poslední
     * příležitost odevzdat zlato hře (a proměnit ho tak v body).
     * Figury, které byly ve hře, jsou v tuto chvíli již zničeny */

    virtual void end( scrap & ) = 0;

    virtual ~player() = default;
};

struct scrap
{
    scrap( terrain,
           std::unique_ptr< player > player_1,
           std::unique_ptr< player > player_2,
           std::unique_ptr< player > player_3 );

    /* Metoda ‹send_to› má jeden parametr typu ‹int›, který urči
     * cílovou pozici (2, 4 nebo 6 podle seznamu výše). Dále:
     *
     *  • metodu lze volat jen pro figury, které se nachází
     *    v domovské pozici a nemají vytyčenou cestu (jinak voláni
     *    skončí výjimkou ‹piece_busy›,
     *  • figura sama určí nejlevnější (nejrychlejší) cestu k cíli a
     *    po této se vydá, bez možnosti dalšího rozhodování ze strany
     *    hráče,
     *  • existuje-li víc nejkratších cest, lze použít libovolnou
     *    z nich. */

    void send_to( piece p, int );

    /* Metoda ‹run_away› figuře uloží, aby z boje utekla. Popis
     * souboje a důsledků útěku je níže. Utéct lze i dodatečně (tzn.
     * hráč může figuru, se kterou hodlá utéct, nějakou dobu držet
     * mimo hru). Nebyla-li figura v souboji vůbec, metoda ale
     * skončí výjimkou ‹no_fight›.  */

    void run_away( piece p );

    /* Byla-li figura vrácena hráči proto, že se setkala s cizí
     * figurou na stejném poli, metoda ‹fight› hru instruuje, že se
     * má hráčova figura pokusit cizí figuru zajmout. Mohou nastat
     * tyto případy:
     *
     *  • obě strany se rozhodnou bojovat: vyhrává ta strana, pro
     *    kterou je rozdíl mezi vlastní energií a soupeřovým životem
     *    větší,
     *  • výherce dostane polovinu energie a třetinu zlata zajaté
     *    figury, která je tímto propuštěna a nemůže být znovu
     *    zajata dříve, než dorazí do cíle (ani třetím hráčem),
     *  • obě strany se rozhodnou utéct: žádné zdroje nemění
     *    vlastníka, pokračuje se jako by k setkání nedošlo,
     *  • jedna strana se rozhodne bojovat, druhá utéct: stejný
     *    efekt jako prohra, ale utíkající strana se pohybuje až do
     *    svého cíle dvojnásobnou rychlostí.
     *
     * Metodu ‹fight› lze použít pouze bezprostředně po setkání –
     * pozdější pokus o její použití vede na výjimku ‹no_fight›,
     * stejně jako pokus o její použití v jiné situaci. */

    void fight( piece );

    /* Metoda ‹take› slouží hráči k odevzdání nasbíraného zlata hře.
     * Takto odevzdané zlato se hráči započítá jako body. Hráč
     * s největším počtem bodů na konci hry vyhrává. */

    void take( resource_gold );

    /* Metoda ‹run› simuluje ‹n›-kolovou hru. V prvním kole hra
     * „rozdá“ všem hráčům jejich figury (voláním metody ‹play›).
     * Každé další kolo probíhá takto:
     *
     *  1. Figury, které jsou ve hře, se pohnou:
     *     
     *     ◦ je-li čekací počítadlo figury na nule, přesune se tato
     *       na další pole na své vytyčené cestě a čekací počítadlo
     *       nastaví na cenu nového pole,
     *     ◦ jinak odečtou jedničku od počítadla a dále nedělají
     *       nic.
     *       
     *  2. Je-li nová hodnota počítadla figury nula a zároveň stojí
     *     na poli se zdrojem, je jí v tuto chvíli:
     *     
     *     ◦ připsán získaný zdroj,
     *     ◦ je vrácena na svou startovní pozici,
     *     ◦ čekací počítadlo je nastaveno na hodnotu této pozice a
     *     ◦ je aktivována metoda ‹play› jejího vlastníka.
     *     
     *  3. Je-li nová hodnota počítadla figury nula a zároveň:
     *     
     *     ◦ se ve stejném poli nachází figura jiného hráče,
     *     ◦ ani jedna z figur nemá dočasnou imunitu,
     *     
     *     dojde k souboji těchto dvou figur – oba dotčení hráči
     *     dostanou možnost rozhodnout o své figuře (jsou aktivovány
     *     jejich metody ‹play›). Samotným soubojem se čekací
     *     počítadlo ani jedné figury nemění. Jsou-li dvě možnosti
     *     souboje, vybere se soupeř s nižším čekacím počítadlem.
     *     Jsou-li počítadla stejná, k žádnému souboji nedojde.
     *
     * Výsledkem metody ‹run› je ukazatel na hráče, který vyhrál,
     * nebo ‹nullptr› došlo-li k remíze mezi vedoucími hráči. */

    int run( int n );
};

int main()
{
    struct dumb : player
    {
        resource_life _life;
        resource_mana _mana;
        resource_gold _gold;

        double score = 0;

        void take( resource_life l ) { _life.take( l ); }
        void take( resource_mana m ) { _mana.take( m ); }
        void take( resource_gold g ) { _gold.take( g ); }

        /* just drops the pieces on the floor right away */
        void play( scrap &, piece ) {}

        void end( scrap &s )
        {
            score = _gold.value();
            s.take( std::move( _gold ) );
        }
    };

    struct run_for_it : dumb
    {
        void play( scrap &s, piece p )
        {
            resource_life l;
            resource_mana m;

            l.take( this->_life, 0.25 );
            m.take( this->_mana, 0.25 );

            p.take( std::move( l ) );
            p.take( std::move( m ) );

            if ( !p.gold() )
                s.send_to( std::move( p ), 6 );
        }
    };

    auto runner = std::make_unique< run_for_it >();
    auto expect_win = runner.get();

    scrap s( terrain(),
             std::make_unique< dumb >(),
             std::move( runner ),
             std::make_unique< dumb >() );

    assert( s.run( 20 ) == expect_win );
    assert( expect_win->score == 0.75 );
    return 0;
}
