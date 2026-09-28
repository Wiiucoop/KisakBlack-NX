#include "cscr_yacc.h"

#include <cstdlib>
#include <malloc.h>
#include "cscr_parsetree.h"
#include "cscr_compiler.h"
#include "cscr_stringlist.h"
#include <server_mp/sv_bot_mp.h>
#include "cscr_parser.h"
#include <client/splitscreen.h>

#include "yacc_structs.h"

int yynerrs;
int yychar;

int yy_init = 1;
int yy_n_chars;
FILE *yyin;
FILE *yyout;
yy_buffer_state *yy_current_buffer;
unsigned __int8 g_parse_user;
unsigned int g_sourcePos;
unsigned int g_out_pos;
char *yy_last_accepting_cpos;
int yy_last_accepting_state;
char *yytext;
unsigned int yyleng;
int yy_start;
char *yy_c_buf_p;
char yy_hold_char;
int yy_did_buffer_switch_on_eof;

stype_t yylval;
sval_u yaccResult;
scriptInstance_t gInst;
sval_u g_dummyVal;

char ch_buf[16386];

int __cdecl yyparse()
{
    void *v1; // esp
    void *v2; // esp
    unsigned int pos; // eax
    sval_u v4; // eax
    sval_u v5; // eax
    sval_u v6; // eax
    sval_u v7; // eax
    sval_u v8; // eax
    sval_u v9; // eax
    sval_u v10; // eax
    sval_u v11; // eax
    sval_u v12; // eax
    sval_u v13; // eax
    sval_u v14; // eax
    sval_u v15; // eax
    sval_u v16; // eax
    //const char *v17; // [esp-4h] [ebp-CECh]
    //int v18; // [esp+0h] [ebp-CE8h] BYREF
    //int v19; // [esp+4h] [ebp-CE4h]
    //int *v20; // [esp+8h] [ebp-CE0h]
    //int *v21; // [esp+Ch] [ebp-CDCh]
    sval_u v22; // [esp+10h] [ebp-CD8h]
    sval_u v23; // [esp+14h] [ebp-CD4h]
    sval_u v24; // [esp+18h] [ebp-CD0h]
    sval_u v25; // [esp+1Ch] [ebp-CCCh]
    sval_u v26; // [esp+20h] [ebp-CC8h]
    sval_u v27; // [esp+24h] [ebp-CC4h]
    sval_u v28; // [esp+28h] [ebp-CC0h]
    sval_u v29; // [esp+2Ch] [ebp-CBCh]
    sval_u v30; // [esp+30h] [ebp-CB8h]
    sval_u v31; // [esp+34h] [ebp-CB4h]
    sval_u v32; // [esp+38h] [ebp-CB0h]
    sval_u v33; // [esp+3Ch] [ebp-CACh]
    sval_u v34; // [esp+40h] [ebp-CA8h]
    sval_u v35; // [esp+44h] [ebp-CA4h]
    sval_u v36; // [esp+48h] [ebp-CA0h]
    sval_u v37; // [esp+4Ch] [ebp-C9Ch]
    sval_u v38; // [esp+50h] [ebp-C98h]
    sval_u v39; // [esp+54h] [ebp-C94h]
    sval_u v40; // [esp+58h] [ebp-C90h]
    sval_u v41; // [esp+5Ch] [ebp-C8Ch]
    sval_u v42; // [esp+60h] [ebp-C88h]
    sval_u v43; // [esp+64h] [ebp-C84h]
    sval_u v44; // [esp+68h] [ebp-C80h]
    sval_u v45; // [esp+6Ch] [ebp-C7Ch]
    sval_u v46; // [esp+70h] [ebp-C78h]
    sval_u v47; // [esp+74h] [ebp-C74h]
    sval_u v48; // [esp+78h] [ebp-C70h]
    sval_u v49; // [esp+7Ch] [ebp-C6Ch]
    sval_u v50; // [esp+80h] [ebp-C68h]
    sval_u v51; // [esp+84h] [ebp-C64h]
    sval_u v52; // [esp+88h] [ebp-C60h]
    sval_u v53; // [esp+8Ch] [ebp-C5Ch]
    sval_u v54; // [esp+90h] [ebp-C58h]
    sval_u v55; // [esp+94h] [ebp-C54h]
    sval_u v56; // [esp+98h] [ebp-C50h]
    sval_u v57; // [esp+9Ch] [ebp-C4Ch]
    sval_u v58; // [esp+A0h] [ebp-C48h]
    sval_u v59; // [esp+A4h] [ebp-C44h]
    sval_u v60; // [esp+A8h] [ebp-C40h]
    sval_u v61; // [esp+ACh] [ebp-C3Ch]
    sval_u v62; // [esp+B0h] [ebp-C38h]
    sval_u v63; // [esp+B4h] [ebp-C34h]
    sval_u v64; // [esp+B8h] [ebp-C30h]
    sval_u v65; // [esp+BCh] [ebp-C2Ch]
    sval_u v66; // [esp+C0h] [ebp-C28h]
    sval_u v67; // [esp+C4h] [ebp-C24h]
    sval_u v68; // [esp+C8h] [ebp-C20h]
    sval_u v69; // [esp+CCh] [ebp-C1Ch]
    sval_u v70; // [esp+D0h] [ebp-C18h]
    sval_u v71; // [esp+D4h] [ebp-C14h]
    sval_u v72; // [esp+D8h] [ebp-C10h]
    sval_u v73; // [esp+DCh] [ebp-C0Ch]
    sval_u v74; // [esp+E0h] [ebp-C08h]
    sval_u v75; // [esp+E4h] [ebp-C04h]
    sval_u v76; // [esp+E8h] [ebp-C00h]
    sval_u v77; // [esp+ECh] [ebp-BFCh]
    sval_u v78; // [esp+F0h] [ebp-BF8h]
    sval_u v79; // [esp+F4h] [ebp-BF4h]
    sval_u v80; // [esp+F8h] [ebp-BF0h]
    sval_u val; // [esp+FCh] [ebp-BECh]
    sval_u v82; // [esp+100h] [ebp-BE8h]
    sval_u appended; // [esp+104h] [ebp-BE4h]
    sval_u v84; // [esp+108h] [ebp-BE0h]
    sval_u v85; // [esp+10Ch] [ebp-BDCh]
    sval_u v86; // [esp+110h] [ebp-BD8h]
    sval_u v87; // [esp+114h] [ebp-BD4h]
    sval_u v88; // [esp+118h] [ebp-BD0h]
    sval_u v89; // [esp+11Ch] [ebp-BCCh]
    sval_u v90; // [esp+120h] [ebp-BC8h]
    sval_u v91; // [esp+124h] [ebp-BC4h]
    sval_u v92; // [esp+128h] [ebp-BC0h]
    sval_u v93; // [esp+12Ch] [ebp-BBCh]
    sval_u val6; // [esp+130h] [ebp-BB8h]
    sval_u v95; // [esp+134h] [ebp-BB4h]
    sval_u v96; // [esp+138h] [ebp-BB0h]
    sval_u v97; // [esp+13Ch] [ebp-BACh]
    sval_u v98; // [esp+140h] [ebp-BA8h]
    sval_u v99; // [esp+144h] [ebp-BA4h]
    sval_u v100; // [esp+148h] [ebp-BA0h]
    sval_u v101; // [esp+14Ch] [ebp-B9Ch]
    sval_u v102; // [esp+150h] [ebp-B98h]
    sval_u v103; // [esp+154h] [ebp-B94h]
    sval_u v104; // [esp+158h] [ebp-B90h]
    sval_u v105; // [esp+15Ch] [ebp-B8Ch]
    sval_u v106; // [esp+160h] [ebp-B88h]
    sval_u v107; // [esp+164h] [ebp-B84h]
    sval_u v108; // [esp+168h] [ebp-B80h]
    sval_u v109; // [esp+16Ch] [ebp-B7Ch]
    sval_u v110; // [esp+170h] [ebp-B78h]
    sval_u v111; // [esp+174h] [ebp-B74h]
    sval_u v112; // [esp+178h] [ebp-B70h]
    sval_u v113; // [esp+17Ch] [ebp-B6Ch]
    sval_u v114; // [esp+180h] [ebp-B68h]
    sval_u v115; // [esp+184h] [ebp-B64h]
    sval_u v116; // [esp+188h] [ebp-B60h]
    sval_u v117; // [esp+18Ch] [ebp-B5Ch]
    sval_u v118; // [esp+190h] [ebp-B58h]
    sval_u v119; // [esp+194h] [ebp-B54h]
    sval_u v120; // [esp+198h] [ebp-B50h]
    sval_u v121; // [esp+19Ch] [ebp-B4Ch]
    sval_u v122; // [esp+1A0h] [ebp-B48h]
    sval_u v123; // [esp+1A4h] [ebp-B44h]
    sval_u v124; // [esp+1A8h] [ebp-B40h]
    sval_u v125; // [esp+1ACh] [ebp-B3Ch]
    sval_u v126; // [esp+1B0h] [ebp-B38h]
    sval_u v127; // [esp+1B4h] [ebp-B34h]
    sval_u v128; // [esp+1B8h] [ebp-B30h]
    sval_u v129; // [esp+1BCh] [ebp-B2Ch]
    sval_u v130; // [esp+1C0h] [ebp-B28h]
    sval_u v131; // [esp+1C4h] [ebp-B24h]
    sval_u v132; // [esp+1C8h] [ebp-B20h]
    sval_u v133; // [esp+1CCh] [ebp-B1Ch]
    sval_u v134; // [esp+1D0h] [ebp-B18h]
    sval_u v135; // [esp+1D4h] [ebp-B14h]
    sval_u v136; // [esp+1D8h] [ebp-B10h]
    sval_u v137; // [esp+1DCh] [ebp-B0Ch]
    sval_u v138; // [esp+1E0h] [ebp-B08h]
    sval_u v139; // [esp+1E4h] [ebp-B04h]
    sval_u v140; // [esp+1E8h] [ebp-B00h]
    sval_u v141; // [esp+1ECh] [ebp-AFCh]
    sval_u v142; // [esp+1F0h] [ebp-AF8h]
    sval_u v143; // [esp+1F4h] [ebp-AF4h]
    sval_u v144; // [esp+1F8h] [ebp-AF0h]
    sval_u v145; // [esp+1FCh] [ebp-AECh]
    sval_u v146; // [esp+200h] [ebp-AE8h]
    sval_u v147; // [esp+204h] [ebp-AE4h]
    sval_u v148; // [esp+208h] [ebp-AE0h]
    sval_u v149; // [esp+20Ch] [ebp-ADCh]
    sval_u v150; // [esp+210h] [ebp-AD8h]
    sval_u v151; // [esp+214h] [ebp-AD4h]
    sval_u v152; // [esp+218h] [ebp-AD0h]
    sval_u v153; // [esp+21Ch] [ebp-ACCh]
    sval_u v154; // [esp+220h] [ebp-AC8h]
    sval_u v155; // [esp+224h] [ebp-AC4h]
    sval_u v156; // [esp+228h] [ebp-AC0h]
    sval_u v157; // [esp+22Ch] [ebp-ABCh]
    sval_u v158; // [esp+230h] [ebp-AB8h]
    sval_u v159; // [esp+234h] [ebp-AB4h]
    sval_u v160; // [esp+238h] [ebp-AB0h]
    sval_u v161; // [esp+23Ch] [ebp-AACh]
    sval_u v162; // [esp+240h] [ebp-AA8h]
    sval_u v163; // [esp+244h] [ebp-AA4h]
    sval_u v164; // [esp+248h] [ebp-AA0h]
    sval_u v165; // [esp+24Ch] [ebp-A9Ch]
    sval_u v166; // [esp+250h] [ebp-A98h]
    sval_u v167; // [esp+254h] [ebp-A94h]
    sval_u v168; // [esp+258h] [ebp-A90h]
    sval_u v169; // [esp+25Ch] [ebp-A8Ch]
    sval_u v170; // [esp+260h] [ebp-A88h]
    sval_u v171; // [esp+264h] [ebp-A84h]
    sval_u v172; // [esp+268h] [ebp-A80h]
    sval_u v173; // [esp+26Ch] [ebp-A7Ch]
    sval_u v174; // [esp+270h] [ebp-A78h]
    sval_u v175; // [esp+274h] [ebp-A74h]
    sval_u v176; // [esp+278h] [ebp-A70h]
    sval_u v177; // [esp+27Ch] [ebp-A6Ch]
    sval_u v178; // [esp+280h] [ebp-A68h]
    sval_u v179; // [esp+284h] [ebp-A64h]
    sval_u v180; // [esp+288h] [ebp-A60h]
    sval_u v181; // [esp+28Ch] [ebp-A5Ch]
    sval_u v182; // [esp+290h] [ebp-A58h]
    sval_u v183; // [esp+294h] [ebp-A54h]
    sval_u v184; // [esp+298h] [ebp-A50h]
    sval_u v185; // [esp+29Ch] [ebp-A4Ch]
    sval_u v186; // [esp+2A0h] [ebp-A48h]
    sval_u v187; // [esp+2A4h] [ebp-A44h]
    sval_u v188; // [esp+2A8h] [ebp-A40h]
    sval_u v189; // [esp+2ACh] [ebp-A3Ch]
    sval_u v190; // [esp+2B0h] [ebp-A38h]
    sval_u v191; // [esp+2B4h] [ebp-A34h]
    sval_u v192; // [esp+2B8h] [ebp-A30h]
    sval_u v193; // [esp+2BCh] [ebp-A2Ch]
    sval_u v194; // [esp+2C0h] [ebp-A28h]
    sval_u v195; // [esp+2C4h] [ebp-A24h]
    sval_u v196; // [esp+2C8h] [ebp-A20h]
    sval_u v197; // [esp+2CCh] [ebp-A1Ch]
    sval_u v198; // [esp+2D0h] [ebp-A18h]
    sval_u v199; // [esp+2D4h] [ebp-A14h]
    sval_u v200; // [esp+2D8h] [ebp-A10h]
    sval_u v201; // [esp+2DCh] [ebp-A0Ch]
    sval_u v202; // [esp+2E0h] [ebp-A08h]
    sval_u v203; // [esp+2E4h] [ebp-A04h]
    sval_u v204; // [esp+2E8h] [ebp-A00h]
    sval_u v205; // [esp+2ECh] [ebp-9FCh]
    sval_u v206; // [esp+2F0h] [ebp-9F8h]
    sval_u v207; // [esp+2F4h] [ebp-9F4h]
    sval_u v208; // [esp+2F8h] [ebp-9F0h]
    sval_u v209; // [esp+2FCh] [ebp-9ECh]
    sval_u v210; // [esp+300h] [ebp-9E8h]
    sval_u v211; // [esp+304h] [ebp-9E4h]
    sval_u v212; // [esp+308h] [ebp-9E0h]
    sval_u v213; // [esp+30Ch] [ebp-9DCh]
    sval_u v214; // [esp+310h] [ebp-9D8h]
    sval_u val1; // [esp+314h] [ebp-9D4h]
    sval_u v216; // [esp+318h] [ebp-9D0h]
    sval_u v217; // [esp+31Ch] [ebp-9CCh]
    sval_u v218; // [esp+320h] [ebp-9C8h]
    sval_u v219; // [esp+324h] [ebp-9C4h]
    sval_u v220; // [esp+328h] [ebp-9C0h]
    sval_u v221; // [esp+32Ch] [ebp-9BCh]
    sval_u v222; // [esp+330h] [ebp-9B8h]
    sval_u v223; // [esp+334h] [ebp-9B4h]
    sval_u v224; // [esp+338h] [ebp-9B0h]
    sval_u v225; // [esp+33Ch] [ebp-9ACh]
    sval_u v226; // [esp+340h] [ebp-9A8h]
    sval_u v227; // [esp+344h] [ebp-9A4h]
    sval_u v228; // [esp+348h] [ebp-9A0h]
    sval_u v229; // [esp+34Ch] [ebp-99Ch]
    sval_u v230; // [esp+350h] [ebp-998h]
    sval_u v231; // [esp+354h] [ebp-994h]
    sval_u v232; // [esp+358h] [ebp-990h]
    sval_u v233; // [esp+35Ch] [ebp-98Ch]
    sval_u v234; // [esp+360h] [ebp-988h]
    sval_u v235; // [esp+364h] [ebp-984h]
    sval_u v236; // [esp+368h] [ebp-980h]
    sval_u v237; // [esp+36Ch] [ebp-97Ch]
    sval_u v238; // [esp+370h] [ebp-978h]
    sval_u v239; // [esp+374h] [ebp-974h]
    sval_u v240; // [esp+378h] [ebp-970h]
    sval_u v241; // [esp+37Ch] [ebp-96Ch]
    sval_u v242; // [esp+380h] [ebp-968h]
    sval_u v243; // [esp+384h] [ebp-964h]
    sval_u v244; // [esp+388h] [ebp-960h]
    sval_u v245; // [esp+38Ch] [ebp-95Ch]
    sval_u v246; // [esp+390h] [ebp-958h]
    sval_u v247; // [esp+394h] [ebp-954h]
    sval_u v248; // [esp+398h] [ebp-950h]
    sval_u v249; // [esp+39Ch] [ebp-94Ch]
    sval_u v250; // [esp+3A0h] [ebp-948h]
    sval_u v251; // [esp+3A4h] [ebp-944h]
    sval_u v252; // [esp+3A8h] [ebp-940h]
    sval_u v253; // [esp+3ACh] [ebp-93Ch]
    sval_u v254; // [esp+3B0h] [ebp-938h]
    sval_u v255; // [esp+3B4h] [ebp-934h]
    sval_u v256; // [esp+3B8h] [ebp-930h]
    sval_u v257; // [esp+3BCh] [ebp-92Ch]
    sval_u v258; // [esp+3C0h] [ebp-928h]
    sval_u v259; // [esp+3C4h] [ebp-924h]
    sval_u v260; // [esp+3C8h] [ebp-920h]
    sval_u v261; // [esp+3CCh] [ebp-91Ch]
    sval_u v262; // [esp+3D0h] [ebp-918h]
    sval_u v263; // [esp+3D4h] [ebp-914h]
    sval_u v264; // [esp+3D8h] [ebp-910h]
    sval_u v265; // [esp+3DCh] [ebp-90Ch]
    sval_u v266; // [esp+3E0h] [ebp-908h]
    sval_u v267; // [esp+3E4h] [ebp-904h]
    sval_u v268; // [esp+3E8h] [ebp-900h]
    sval_u v269; // [esp+3ECh] [ebp-8FCh]
    sval_u v270; // [esp+3F0h] [ebp-8F8h]
    sval_u v271; // [esp+3F4h] [ebp-8F4h]
    sval_u v272; // [esp+3F8h] [ebp-8F0h]
    sval_u v273; // [esp+3FCh] [ebp-8ECh]
    sval_u v274; // [esp+400h] [ebp-8E8h]
    sval_u v275; // [esp+404h] [ebp-8E4h]
    sval_u v276; // [esp+408h] [ebp-8E0h]
    sval_u v277; // [esp+40Ch] [ebp-8DCh]
    sval_u v278; // [esp+410h] [ebp-8D8h]
    sval_u v279; // [esp+414h] [ebp-8D4h]
    sval_u v280; // [esp+418h] [ebp-8D0h]
    sval_u v281; // [esp+41Ch] [ebp-8CCh]
    sval_u v282; // [esp+420h] [ebp-8C8h]
    sval_u v283; // [esp+424h] [ebp-8C4h]
    sval_u v284; // [esp+428h] [ebp-8C0h]
    sval_u v285; // [esp+42Ch] [ebp-8BCh]
    sval_u v286; // [esp+430h] [ebp-8B8h]
    sval_u v287; // [esp+434h] [ebp-8B4h]
    sval_u v288; // [esp+438h] [ebp-8B0h]
    sval_u v289; // [esp+43Ch] [ebp-8ACh]
    sval_u v290; // [esp+440h] [ebp-8A8h]
    sval_u v291; // [esp+444h] [ebp-8A4h]
    sval_u v292; // [esp+448h] [ebp-8A0h]
    sval_u v293; // [esp+44Ch] [ebp-89Ch]
    sval_u v294; // [esp+450h] [ebp-898h]
    sval_u v295; // [esp+454h] [ebp-894h]
    sval_u v296; // [esp+458h] [ebp-890h]
    sval_u v297; // [esp+45Ch] [ebp-88Ch]
    sval_u v298; // [esp+460h] [ebp-888h]
    sval_u v299; // [esp+464h] [ebp-884h]
    sval_u v300; // [esp+468h] [ebp-880h]
    sval_u v301; // [esp+46Ch] [ebp-87Ch]
    sval_u v302; // [esp+470h] [ebp-878h]
    sval_u v303; // [esp+474h] [ebp-874h]
    sval_u v304; // [esp+478h] [ebp-870h]
    sval_u v305; // [esp+47Ch] [ebp-86Ch]
    sval_u v306; // [esp+480h] [ebp-868h]
    sval_u v307; // [esp+484h] [ebp-864h]
    sval_u v308; // [esp+488h] [ebp-860h]
    sval_u v309; // [esp+48Ch] [ebp-85Ch]
    sval_u v310; // [esp+490h] [ebp-858h]
    sval_u v311; // [esp+494h] [ebp-854h]
    sval_u v312; // [esp+498h] [ebp-850h]
    sval_u v313; // [esp+49Ch] [ebp-84Ch]
    sval_u v314; // [esp+4A0h] [ebp-848h]
    sval_u v315; // [esp+4A4h] [ebp-844h]
    sval_u val3; // [esp+4A8h] [ebp-840h]
    sval_u val4; // [esp+4ACh] [ebp-83Ch]
    sval_u val5; // [esp+4B0h] [ebp-838h]
    sval_u v319; // [esp+4B4h] [ebp-834h]
    sval_u val2; // [esp+4B8h] [ebp-830h]
    sval_u v321; // [esp+4BCh] [ebp-82Ch]
    sval_u v322; // [esp+4C0h] [ebp-828h]
    sval_u v323; // [esp+4C4h] [ebp-824h]
    sval_u v324; // [esp+4C8h] [ebp-820h]
    sval_u v325; // [esp+4CCh] [ebp-81Ch]
    int size; // [esp+4D0h] [ebp-818h]
    stype_t *yyvs1; // [esp+4D4h] [ebp-814h]
    __int16 *yyss1; // [esp+4D8h] [ebp-810h]
    int yyerrstatus; // [esp+4DCh] [ebp-80Ch]
    int yyfree_stacks; // [esp+4E0h] [ebp-808h]
    int yyn; // [esp+4E4h] [ebp-804h]
    int yystate; // [esp+4E8h] [ebp-800h]
    __int16 *yyssp; // [esp+4ECh] [ebp-7FCh] BYREF
    __int16 yyssa[200]; // [esp+4F0h] [ebp-7F8h] BYREF
    stype_t *yyvs; // [esp+688h] [ebp-660h]
    int yychar1; // [esp+68Ch] [ebp-65Ch]
    __int16 *yyss; // [esp+690h] [ebp-658h]
    stype_t yyval; // [esp+694h] [ebp-654h]
    int yylen; // [esp+69Ch] [ebp-64Ch]
    stype_t yyvsa[200]; // [esp+6A0h] [ebp-648h] BYREF
    stype_t *yyvsp; // [esp+CE0h] [ebp-8h]
    int yystacksize; // [esp+CE4h] [ebp-4h]

    yychar1 = 0;

    yyss = yyssa;
    yyvs = yyvsa;

    yystacksize = 200;
    yyfree_stacks = 0;
    yystate = 0;
    yyerrstatus = 0;
    yynerrs = 0;
    yyval.pos = 0;
    yychar = -2;

    yyssp = yyss;
    yyvsp = yyvs;

    while (1)
    {
    yynewstate:
        *++yyssp = yystate;
        if (yyssp >= &yyss[yystacksize - 1])
        {
            yyvs1 = yyvs;
            yyss1 = yyss;
            size = yyssp - yyss + 1;
            if (yystacksize >= 10000)
            {
                //v17 = "parser stack overflow";
                yyerror();
                if (yyfree_stacks)
                {
                    free(yyss);
                    free(yyvs);
                }
                return 2;
            }
            yystacksize *= 2;
            if (yystacksize > 10000)
                yystacksize = 10000;

            //v1 = alloca(sizeof(short) * yystacksize);
            //v21 = &v18;
            //yyss = (__int16 *)&v18;
            yyss = (short *)alloca(sizeof(short) * yystacksize);
            memcpy(yyss, yyss1, sizeof(short) * size);

            //v2 = alloca(sizeof(stype_t) * yystacksize);
            //v20 = &v18;
            //yyvs = (stype_t *)&v18;
            yyvs = (stype_t *)alloca(sizeof(stype_t) * yystacksize);
            memcpy(yyvs, yyvs1, sizeof(stype_t) * size);

            yyssp = &yyss[size - 1];
            yyvsp = &yyvs[size - 1];

            if (yyssp >= &yyss[yystacksize - 1])
                goto yyabortlab;
        }
        yyn = yypact[yystate];
        if (yyn == -32768)
            break;
        if (yychar == -2)
            yychar = yylex();
        if (yychar > 0)
        {
            yychar1 = (unsigned int)yychar > 0x159 ? 120 : yytranslate[yychar];
        }
        else
        {
            yychar1 = 0;
            yychar = 0;
        }
        yyn += yychar1;
        if ((unsigned int)yyn >= 0x59C || yycheck[yyn] != yychar1)
            break;
        yyn = yytable[yyn];
        if (yyn >= 0)
        {
            if (!yyn)
                goto yyerrlab;
            if (yyn == 267)
                goto yyacceptlab;
            if (yychar)
                yychar = -2;
            *++yyvsp = yylval;
            if (yyerrstatus)
                --yyerrstatus;
            yystate = yyn;
        }
        else
        {
            if (yyn == -32768)
                goto yyerrlab;
            yyn = -yyn;
        yyreduce:
            yylen = yyr2[yyn];
            if (yylen > 0)
            {
                pos = yyvsp[1 - yylen].pos;
                yyval.val = yyvsp[1 - yylen].val;   // nx-port: the whole value ($$ = $1); it is often a node pointer
                yyval.pos = pos;
            }
            //v18 = yyn - 1; // assuming this is jumptable asm prep
            switch (yyn)
            {
            case 1:
                v325 = node2_(gInst, yyvsp[-1].val, yyvsp->val);
                yaccResult = v325;
                break;
            case 2:
                v324 = node1(gInst, ENUM_expression, yyvsp->val);
                yaccResult = v324;
                break;
            case 3:
                v323 = node1(gInst, ENUM_statement, yyvsp->val);
                yaccResult = v323;
                break;
            case 4:
                v322 = node1(gInst, ENUM_statement, yyvsp->val);
                yaccResult = v322;
                break;
            case 5:
                val2 = node_pos(yyvsp->pos);
                v321 = node2(gInst, ENUM_primitive_expression, yyvsp->val, val2);
                yyval.val = v321;
                break;
            case 6:
                val5 = node_pos(yyvsp[-1].pos);
                val4 = node_pos(yyvsp->pos);
                val3 = node_pos(yyvsp[-2].pos);
                v319 = node5(gInst, ENUM_bool_or, yyvsp[-2].val, yyvsp->val, val3, val4, val5);
                yyval.val = v319;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 7:
                v314 = node_pos(yyvsp[-1].pos);
                v313 = node_pos(yyvsp->pos);
                v312 = node_pos(yyvsp[-2].pos);
                v315 = node5(gInst, ENUM_bool_and, yyvsp[-2].val, yyvsp->val, v312, v313, v314);
                yyval.val = v315;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 8:
                v310 = node_pos(yyvsp[-1].pos);
                v309 = node1_(gInst, 102);
                v311 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v309, v310);
                yyval.val = v311;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 9:
                v307 = node_pos(yyvsp[-1].pos);
                v306 = node1_(gInst, 103);
                v308 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v306, v307);
                yyval.val = v308;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 10:
                v304 = node_pos(yyvsp[-1].pos);
                v303 = node1_(gInst, 104);
                v305 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v303, v304);
                yyval.val = v305;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 11:
                v301 = node_pos(yyvsp[-1].pos);
                v300 = node1_(gInst, 105);
                v302 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v300, v301);
                yyval.val = v302;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 12:
                v298 = node_pos(yyvsp[-1].pos);
                v297 = node1_(gInst, 106);
                v299 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v297, v298);
                yyval.val = v299;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 13:
                v295 = node_pos(yyvsp[-1].pos);
                v294 = node1_(gInst, 107);
                v296 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v294, v295);
                yyval.val = v296;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 14:
                v292 = node_pos(yyvsp[-1].pos);
                v291 = node1_(gInst, 108);
                v293 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v291, v292);
                yyval.val = v293;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 15:
                v289 = node_pos(yyvsp[-1].pos);
                v288 = node1_(gInst, 109);
                v290 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v288, v289);
                yyval.val = v290;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 16:
                v286 = node_pos(yyvsp[-1].pos);
                v285 = node1_(gInst, 110);
                v287 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v285, v286);
                yyval.val = v287;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 17:
                v283 = node_pos(yyvsp[-1].pos);
                v282 = node1_(gInst, 111);
                v284 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v282, v283);
                yyval.val = v284;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 18:
                v280 = node_pos(yyvsp[-1].pos);
                v279 = node1_(gInst, 112);
                v281 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v279, v280);
                yyval.val = v281;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 19:
                v277 = node_pos(yyvsp[-1].pos);
                v276 = node1_(gInst, 113);
                v278 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v276, v277);
                yyval.val = v278;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 20:
                v274 = node_pos(yyvsp[-1].pos);
                v273 = node1_(gInst, 114);
                v275 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v273, v274);
                yyval.val = v275;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 21:
                v271 = node_pos(yyvsp[-1].pos);
                v270 = node1_(gInst, 115);
                v272 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v270, v271);
                yyval.val = v272;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 22:
                v268 = node_pos(yyvsp[-1].pos);
                v267 = node1_(gInst, 116);
                v269 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v267, v268);
                yyval.val = v269;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 23:
                v265 = node_pos(yyvsp[-1].pos);
                v264 = node1_(gInst, 117);
                v266 = node4(gInst, ENUM_binary, yyvsp[-2].val, yyvsp->val, v264, v265);
                yyval.val = v266;
                yyval.pos = yyvsp[-1].pos;
                break;
            case 24:
                v262 = node_pos(yyvsp->pos);
                v263 = node2(gInst, ENUM_bool_not, yyvsp->val, v262);
                yyval.val = v263;
                break;
            case 25:
                v260 = node_pos(yyvsp->pos);
                v261 = node2(gInst, ENUM_bool_complement, yyvsp->val, v260);
                yyval.val = v261;
                break;
            case 26:
                v259 = node1(gInst, ENUM_expression, yyvsp->val);
                yyval.val = v259;
                break;
            case 27:
                v258 = node0(gInst, ENUM_NOP);
                yyval.val = v258;
                break;
            case 28:
            case 29:
                v4.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v4;
                yyval.val = yyvsp->val;   // nx-port: the whole value
                break;
            case 30:
                v5.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v5;
                v256 = node_pos(yyvsp[-2].pos);
                v257 = node3(gInst, ENUM_far_function, yyvsp[-2].val, yyvsp->val, v256);
                yyval.val = v257;
                ++gScrCompilePub[gInst].far_function_count;
                break;
            case 31:
                v6.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v6;
                v254 = node_pos(yyvsp->pos);
                v255 = node2(gInst, ENUM_local_function, yyvsp->val, v254);
                yyval.val = v255;
                break;
            case 32:
                v7.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v7;
                v252 = node_pos(yyvsp[-2].pos);
                v253 = node3(gInst, ENUM_far_function, yyvsp[-2].val, yyvsp->val, v252);
                yyval.val = v253;
                yyval.pos = yyvsp[-1].pos;
                ++gScrCompilePub[gInst].far_function_count;
                break;
            case 33:
                v8.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v8;
                v250 = node_pos(yyvsp[-1].pos);
                v251 = node2(gInst, ENUM_local_function, yyvsp->val, v250);
                yyval.val = v251;
                break;
            case 34:
                v248 = node_pos(yyvsp->pos);
                v249 = node2(gInst, ENUM_function, yyvsp->val, v248);
                yyval.val = v249;
                break;
            case 35:
                v246 = node_pos(yyvsp[-2].pos);
                v247 = node2(gInst, ENUM_function_pointer, yyvsp[-2].val, v246);
                yyval.val = v247;
                break;
            case 36:
                v244 = node_pos(yyvsp->pos);
                v245 = node2(gInst, ENUM_script_call, yyvsp->val, v244);
                yyval.val = v245;
                break;
            case 37:
                v242 = node_pos(yyvsp->pos);
                v241 = node_pos(yyvsp[-1].pos);
                v243 = node3(gInst, ENUM_script_thread_call, yyvsp->val, v241, v242);
                yyval.val = v243;
                yyval.pos = yyvsp->pos;
                break;
            case 38:
                v239 = node_pos(yyvsp[-2].pos);
                v240 = node3(gInst, ENUM_call, yyvsp[-3].val, yyvsp[-1].val, v239);
                yyval.val = v240;
                yyval.pos = yyvsp[-2].pos;
                break;
            case 39:
                v237 = node_pos(yyvsp[-2].pos);
                v236 = node_pos(yyvsp[-4].pos);
                v238 = node5(gInst, ENUM_method, yyvsp[-4].val, yyvsp[-3].val, yyvsp[-1].val, v236, v237);
                yyval.val = v238;
                yyval.pos = yyvsp[-2].pos;
                break;
            case 40:
                v234 = node_pos(yyvsp[-2].pos);
                v235 = node2(gInst, ENUM_expression_list, yyvsp[-1].val, v234);
                yyval.val = v235;
                break;
            case 41:
                v232 = node_pos(yyvsp->pos);
                v233 = node2(gInst, ENUM_integer, yyvsp->val, v232);
                yyval.val = v233;
                break;
            case 42:
                v230 = node_pos(yyvsp->pos);
                v231 = node2(gInst, ENUM_float, yyvsp->val, v230);
                yyval.val = v231;
                break;
            case 43:
                v228 = node_pos(yyvsp[-1].pos);
                v229 = node2(gInst, ENUM_minus_integer, yyvsp->val, v228);
                yyval.val = v229;
                break;
            case 44:
                v226 = node_pos(yyvsp[-1].pos);
                v227 = node2(gInst, ENUM_minus_float, yyvsp->val, v226);
                yyval.val = v227;
                break;
            case 45:
                v224 = node_pos(yyvsp->pos);
                v225 = node2(gInst, ENUM_string, yyvsp->val, v224);
                yyval.val = v225;
                break;
            case 46:
                v222 = node_pos(yyvsp->pos);
                v223 = node2(gInst, ENUM_istring, yyvsp->val, v222);
                yyval.val = v223;
                break;
            case 47:
                v220 = node_pos(yyvsp->pos);
                v221 = node2(gInst, ENUM_integer, yyvsp->val, v220);
                yyval.val = v221;
                break;
            case 48:
                v219 = node1(gInst, ENUM_call_expression, yyvsp->val);
                yyval.val = v219;
                break;
            case 49:
                v217 = node_pos(yyvsp->pos);
                v218 = node2(gInst, ENUM_variable, yyvsp->val, v217);
                yyval.val = v218;
                break;
            case 50:
                val1 = node_pos(yyvsp->pos);
                v216 = node1(gInst, ENUM_undefined, val1);
                yyval.val = v216;
                break;
            case 51:
                v213 = node_pos(yyvsp->pos);
                v214 = node1(gInst, ENUM_self, v213);
                yyval.val = v214;
                break;
            case 52:
                v211 = node_pos(yyvsp->pos);
                v212 = node1(gInst, ENUM_level, v211);
                yyval.val = v212;
                break;
            case 53:
                v209 = node_pos(yyvsp->pos);
                v210 = node1(gInst, ENUM_game, v209);
                yyval.val = v210;
                break;
            case 54:
                v207 = node_pos(yyvsp->pos);
                v208 = node1(gInst, ENUM_anim, v207);
                yyval.val = v208;
                break;
            case 55:
                v205 = node_pos(yyvsp[-1].pos);
                v206 = node2(gInst, ENUM_size_field, yyvsp[-1].val, v205);
                yyval.val = v206;
                yyval.pos = yyvsp->pos;
                break;
            case 56:
                v203 = node_pos(yyvsp->pos);
                v204 = node2(gInst, ENUM_function, yyvsp->val, v203);
                yyval.val = v204;
                break;
            case 57:
                v201 = node_pos(yyvsp[-1].pos);
                v202 = node1(gInst, ENUM_empty_array, v201);
                yyval.val = v202;
                break;
            case 58:
                v9.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v9;
                v199 = node_pos(yyvsp->pos);
                v200 = node2(gInst, ENUM_animation, yyvsp->val, v199);
                yyval.val = v200;
                break;
            case 59:
                v197 = node_pos(yyvsp->pos);
                v198 = node1(gInst, ENUM_false, v197);
                yyval.val = v198;
                break;
            case 60:
                v195 = node_pos(yyvsp->pos);
                v196 = node1(gInst, ENUM_true, v195);
                yyval.val = v196;
                break;
            case 61:
                v193 = node_pos(yyvsp->pos);
                v194 = node1(gInst, ENUM_animtree, v193);
                yyval.val = v194;
                break;
            case 62:
                v191 = node_pos(yyvsp[-3].pos);
                v192 = node3(gInst, ENUM_breakon, yyvsp[-4].val, yyvsp[-1].val, v191);
                yyval.val = v192;
                break;
            case 63:
                v10.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v10;
                v189 = node_pos(yyvsp[-2].pos);
                v190 = node3(gInst, ENUM_field_variable, yyvsp[-2].val, yyvsp->val, v189);
                yyval.val = v190;
                yyval.pos = yyvsp->pos;
                break;
            case 64:
                v187 = node_pos(yyvsp[-1].pos);
                v186 = node_pos(yyvsp[-3].pos);
                v188 = node4(gInst, ENUM_array_variable, yyvsp[-3].val, yyvsp[-1].val, v186, v187);
                yyval.val = v188;
                yyval.pos = yyvsp[-2].pos;
                break;
            case 65:
                v11.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v11;
                v184 = node_pos(yyvsp->pos);
                v185 = node2(gInst, ENUM_local_variable, yyvsp->val, v184);
                yyval.val = v185;
                break;
            case 66:
                v12.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v12;
                v182 = node_pos(yyvsp[-1].pos);
                v183 = node2(gInst, ENUM_object, yyvsp->val, v182);
                yyval.val = v183;
                break;
            case 67:
                v180 = node_pos(yyvsp[-2].pos);
                v181 = node2(gInst, ENUM_self_field, yyvsp[-2].val, v180);
                yyval.val = v181;
                yyval.pos = yyvsp->pos;
                break;
            case 68:
                v178 = node_pos(yyvsp->pos);
                v177 = node_pos(yyvsp[-1].pos);
                v179 = node4(gInst, ENUM_assignment, yyvsp[-2].val, yyvsp->val, v177, v178);
                yyval.val = v179;
                break;
            case 69:
                v175 = node_pos(yyvsp[-1].pos);
                v176 = node2(gInst, ENUM_return, yyvsp->val, v175);
                yyval.val = v176;
                break;
            case 70:
                v173 = node_pos(yyvsp->pos);
                v174 = node1(gInst, ENUM_return2, v173);
                yyval.val = v174;
                break;
            case 71:
                v171 = node_pos(yyvsp[-1].pos);
                v170 = node_pos(yyvsp->pos);
                v172 = node3(gInst, ENUM_wait, yyvsp->val, v170, v171);
                yyval.val = v172;
                break;
            case 72:
                v168 = node_pos(yyvsp[-1].pos);
                v169 = node2(gInst, ENUM_inc, yyvsp[-1].val, v168);
                yyval.val = v169;
                break;
            case 73:
                v166 = node_pos(yyvsp[-1].pos);
                v167 = node2(gInst, ENUM_dec, yyvsp[-1].val, v166);
                yyval.val = v167;
                break;
            case 74:
                v164 = node_pos(yyvsp[-1].pos);
                v163 = node1_(gInst, 102);
                v165 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v163, v164);
                yyval.val = v165;
                break;
            case 75:
                v161 = node_pos(yyvsp[-1].pos);
                v160 = node1_(gInst, 103);
                v162 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v160, v161);
                yyval.val = v162;
                break;
            case 76:
                v158 = node_pos(yyvsp[-1].pos);
                v157 = node1_(gInst, 104);
                v159 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v157, v158);
                yyval.val = v159;
                break;
            case 77:
                v155 = node_pos(yyvsp[-1].pos);
                v154 = node1_(gInst, 111);
                v156 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v154, v155);
                yyval.val = v156;
                break;
            case 78:
                v152 = node_pos(yyvsp[-1].pos);
                v151 = node1_(gInst, 112);
                v153 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v151, v152);
                yyval.val = v153;
                break;
            case 79:
                v149 = node_pos(yyvsp[-1].pos);
                v148 = node1_(gInst, 113);
                v150 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v148, v149);
                yyval.val = v150;
                break;
            case 80:
                v146 = node_pos(yyvsp[-1].pos);
                v145 = node1_(gInst, 114);
                v147 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v145, v146);
                yyval.val = v147;
                break;
            case 81:
                v143 = node_pos(yyvsp[-1].pos);
                v142 = node1_(gInst, 115);
                v144 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v142, v143);
                yyval.val = v144;
                break;
            case 82:
                v140 = node_pos(yyvsp[-1].pos);
                v139 = node1_(gInst, 116);
                v141 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v139, v140);
                yyval.val = v141;
                break;
            case 83:
                v137 = node_pos(yyvsp[-1].pos);
                v136 = node1_(gInst, 117);
                v138 = node4(gInst, ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, v136, v137);
                yyval.val = v138;
                break;
            case 84:
                v134 = node_pos(yyvsp[-3].pos);
                v133 = node_pos(yyvsp[-4].pos);
                v135 = node4(gInst, ENUM_waittill, yyvsp[-4].val, yyvsp[-1].val, v133, v134);
                yyval.val = v135;
                break;
            case 85:
                v131 = node_pos(yyvsp[-3].pos);
                v130 = node_pos(yyvsp[-4].pos);
                v132 = node4(gInst, ENUM_waittillmatch, yyvsp[-4].val, yyvsp[-1].val, v130, v131);
                yyval.val = v132;
                break;
            case 86:
                v128 = node_pos(yyvsp->pos);
                v129 = node1(gInst, ENUM_waittillFrameEnd, v128);
                yyval.val = v129;
                break;
            case 87:
                v126 = node_pos(yyvsp[-3].pos);
                v125 = node_pos(yyvsp[-4].pos);
                v127 = node4(gInst, ENUM_notify, yyvsp[-4].val, yyvsp[-1].val, v125, v126);
                yyval.val = v127;
                break;
            case 88:
                v123 = node_pos(yyvsp[-1].pos);
                v122 = node_pos(yyvsp[-4].pos);
                v124 = node4(gInst, ENUM_endon, yyvsp[-4].val, yyvsp[-1].val, v122, v123);
                yyval.val = v124;
                break;
            case 89:
                v120 = node_pos(yyvsp->pos);
                v121 = node1(gInst, ENUM_break, v120);
                yyval.val = v121;
                break;
            case 90:
                v118 = node_pos(yyvsp->pos);
                v119 = node1(gInst, ENUM_continue, v118);
                yyval.val = v119;
                break;
            case 91:
                v116 = node_pos(yyvsp->pos);
                v117 = node1(gInst, ENUM_breakpoint, v116);
                yyval.val = v117;
                break;
            case 92:
                v114 = node_pos(yyvsp[-3].pos);
                v115 = node2(gInst, ENUM_prof_begin, yyvsp[-1].val, v114);
                yyval.val = v115;
                break;
            case 93:
                v112 = node_pos(yyvsp[-3].pos);
                v113 = node2(gInst, ENUM_prof_end, yyvsp[-1].val, v112);
                yyval.val = v113;
                break;
            case 94:
                v111 = node1(gInst, ENUM_call_expression_statement, yyvsp->val);
                yyval.val = v111;
                break;
            case 96:
                v110 = node0(gInst, ENUM_NOP);
                yyval.val = v110;
                break;
            case 99:
                v108 = node_pos(yyvsp->pos);
                v107 = node_pos(yyvsp[-2].pos);
                v109 = node3(gInst, ENUM_statement_list, yyvsp[-1].val, v107, v108);
                yyval.val = v109;
                break;
            case 100:
                v105 = node_pos(yyvsp[-2].pos);
                v106 = node4(gInst, ENUM_if, yyvsp[-2].val, yyvsp->val, v105, g_dummyVal);
                yyval.val = v106;
                break;
            case 101:
                v103 = node_pos(yyvsp[-1].pos);
                v102 = node_pos(yyvsp[-4].pos);
                v104 = node7(
                    gInst,
                    ENUM_if_else,
                    yyvsp[-4].val,
                    yyvsp[-2].val,
                    yyvsp->val,
                    v102,
                    v103,
                    g_dummyVal,
                    g_dummyVal);
                yyval.val = v104;
                break;
            case 102:
                v100 = node_pos(yyvsp[-2].pos);
                v99 = node_pos(yyvsp[-6].pos);
                v101 = node7(
                    gInst,
                    ENUM_if_else,
                    yyvsp[-6].val,
                    yyvsp[-3].val,
                    yyvsp[-1].val,
                    v99,
                    v100,
                    g_dummyVal,
                    g_dummyVal);
                yyval.val = v101;
                break;
            case 103:
                v97 = node_pos(yyvsp[-4].pos);
                v96 = node_pos(yyvsp[-2].pos);
                v98 = node5(gInst, ENUM_while, yyvsp[-2].val, yyvsp->val, v96, v97, g_dummyVal);
                yyval.val = v98;
                break;
            case 104:
                val6 = node_pos(yyvsp[-7].pos);
                v93 = node_pos(yyvsp[-4].pos);
                v95 = node8(
                    gInst,
                    ENUM_for,
                    yyvsp[-5].val,
                    yyvsp[-4].val,
                    yyvsp[-2].val,
                    yyvsp->val,
                    v93,
                    val6,
                    g_dummyVal,
                    g_dummyVal);
                yyval.val = v95;
                break;
            case 105:
                v91 = node_pos(yyvsp[-4].pos);
                v92 = node3(gInst, ENUM_switch, yyvsp[-4].val, yyvsp[-1].val, v91);
                yyval.val = v92;
                break;
            case 106:
                v89 = node_pos(yyvsp[-2].pos);
                v90 = node3(gInst, ENUM_developer_statement_list, yyvsp[-1].val, v89, g_dummyVal);
                yyval.val = v90;
                break;
            case 107:
                v88 = node0(gInst, ENUM_NOP);
                yyval.val = v88;
                break;
            case 108:
                v86 = node_pos(yyvsp[-2].pos);
                v87 = node3(gInst, ENUM_case, yyvsp[-1].val, v86, g_dummyVal);
                yyval.val = v87;
                break;
            case 109:
                v84 = node_pos(yyvsp[-1].pos);
                v85 = node2(gInst, ENUM_default, v84, g_dummyVal);
                yyval.val = v85;
                break;
            case 111:
                appended = append_node(gInst, yyvsp[-1].val, yyvsp->val);
                yyval.val = appended;
                break;
            case 112:
                val = node0(gInst, ENUM_NOP);
                v82 = linked_list_end(gInst, val);
                yyval.val = v82;
                break;
            case 113:
                v78 = node_pos(yyvsp->pos);
                v79 = node2_(gInst, yyvsp->val, v78);
                v80 = prepend_node(gInst, v79, yyvsp[-2].val);
                yyval.val = v80;
                break;
            case 114:
                v76 = node0(gInst, ENUM_NOP);
                v74 = node_pos(yyvsp->pos);
                v75 = node2_(gInst, yyvsp->val, v74);
                v77 = prepend_node(gInst, v75, v76);
                yyval.val = v77;
                break;
            case 116:
                v73 = node0(gInst, ENUM_NOP);
                yyval.val = v73;
                break;
            case 117:
                v13.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v13;
                v70 = node_pos(yyvsp->pos);
                v71 = node2_(gInst, yyvsp->val, v70);
                v72 = append_node(gInst, yyvsp[-2].val, v71);
                yyval.val = v72;
                break;
            case 118:
                v14.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v14;
                v67 = node_pos(yyvsp->pos);
                v68 = node2_(gInst, yyvsp->val, v67);
                v65 = node0(gInst, ENUM_NOP);
                v66 = linked_list_end(gInst, v65);
                v69 = append_node(gInst, v66, v68);
                yyval.val = v69;
                break;
            case 120:
                v63 = node0(gInst, ENUM_NOP);
                v64 = linked_list_end(gInst, v63);
                yyval.val = v64;
                break;
            case 121:
                v15.stringValue = LowerCase(yyvsp->val.stringValue);
                yyvsp->val = v15;
                v60 = node_pos(yyvsp->pos);
                v61 = node2_(gInst, yyvsp->val, v60);
                v62 = append_node(gInst, yyvsp[-2].val, v61);
                yyval.val = v62;
                break;
            case 122:
                v57 = node_pos(yyvsp->pos);
                v58 = node2_(gInst, yyvsp->val, v57);
                v55 = node0(gInst, ENUM_NOP);
                v56 = linked_list_end(gInst, v55);
                v59 = append_node(gInst, v56, v58);
                yyval.val = v59;
                break;
            case 123:
                v52 = node_pos(yyvsp->pos);
                v53 = node2_(gInst, yyvsp->val, v52);
                v54 = append_node(gInst, yyvsp[-2].val, v53);
                yyval.val = v54;
                break;
            case 124:
                v49 = node_pos(yyvsp->pos);
                v50 = node2_(gInst, yyvsp->val, v49);
                v47 = node0(gInst, ENUM_NOP);
                v48 = linked_list_end(gInst, v47);
                v51 = append_node(gInst, v48, v50);
                yyval.val = v51;
                break;
            case 125:
                v44 = node_pos(yyvsp->pos);
                v45 = node2_(gInst, yyvsp->val, v44);
                v46 = prepend_node(gInst, v45, yyvsp[-2].val);
                yyval.val = v46;
                break;
            case 126:
                v42 = node0(gInst, ENUM_NOP);
                v40 = node_pos(yyvsp->pos);
                v41 = node2_(gInst, yyvsp->val, v40);
                v43 = prepend_node(gInst, v41, v42);
                yyval.val = v43;
                break;
            case 127:
                v16.stringValue = LowerCase(yyvsp[-6].val.stringValue);
                yyvsp[-6].val = v16;
                v38 = node_pos(yyvsp->pos);
                v37 = node_pos(yyvsp[-6].pos);
                v39 = node6(gInst, ENUM_thread, yyvsp[-6].val, yyvsp[-4].val, yyvsp[-1].val, v37, v38, g_dummyVal);
                yyval.val = v39;
                break;
            case 128:
                v35 = node_pos(yyvsp[-2].pos);
                v34 = node_pos(yyvsp[-4].pos);
                v36 = node3(gInst, ENUM_usingtree, yyvsp[-2].val, v34, v35);
                yyval.val = v36;
                break;
            case 129:
                v32 = node_pos(yyvsp->pos);
                v33 = node1(gInst, ENUM_begin_developer_thread, v32);
                yyval.val = v33;
                break;
            case 130:
                v30 = node_pos(yyvsp->pos);
                v31 = node1(gInst, ENUM_end_developer_thread, v30);
                yyval.val = v31;
                break;
            case 131:
                v29 = append_node(gInst, yyvsp[-1].val, yyvsp->val);
                yyval.val = v29;
                break;
            case 132:
                v27 = node0(gInst, ENUM_NOP);
                v28 = linked_list_end(gInst, v27);
                yyval.val = v28;
                break;
            case 133:
                v25 = node_pos(yyvsp[-1].pos);
                v26 = node2(gInst, ENUM_include, yyvsp->val, v25);
                yyval.val = v26;
                ++gScrCompilePub[gInst].far_function_count;
                break;
            case 134:
                v24 = append_node(gInst, yyvsp[-2].val, yyvsp[-1].val);
                yyval.val = v24;
                break;
            case 135:
                v22 = node0(gInst, ENUM_NOP);
                v23 = linked_list_end(gInst, v22);
                yyval.val = v23;
                break;
            default:
                break;
            }
            yyvsp -= yylen;
            yyssp -= yylen;
            *++yyvsp = yyval;

            yyn = yyr1[yyn];

#define YYNTOKENS 92
            //yystate = *yyssp + yypact[yyn + 176]; // LWSS CHANGE
            yystate = *yyssp + yypgoto[yyn - YYNTOKENS];

            //if ((unsigned int)yystate < 0x59C && yycheck[yystate] == *yyssp)
            //    yystate = yytable[yystate];
            //else
            //    yystate = yydefact[yyn + 176];

            if ((unsigned int)yystate < YYTABLESIZE && yycheck[yystate] == *yyssp)
                yystate = yytable[yystate];
            else
                yystate = yydefgoto[yyn - YYNTOKENS];
        }
    }
    yyn = yydefact[yystate];
    if (yyn)
        goto yyreduce;
yyerrlab:
    if (!yyerrstatus)
    {
        ++yynerrs;
        //v17 = "parse error";
        yyerror();
    }
    if (yyerrstatus == 3)
    {
        if (!yychar)
        {
        yyabortlab:
            if (yyfree_stacks)
            {
                free(yyss);
                free(yyvs);
            }
            return 1;
        }
        yychar = -2;
    }
    yyerrstatus = 3;
    while (2)
    {
        yyn = yypact[yystate];
        if (yyn == -32768)
            goto yyerrpop;
        if ((unsigned int)++yyn >= 0x59C || yycheck[yyn] != 1)
            goto yyerrpop;
        yyn = yytable[yyn];
        if (yyn < 0)
        {
            if (yyn != -32768)
            {
                yyn = -yyn;
                goto yyreduce;
            }
            goto yyerrpop;
        }
        if (!yyn)
        {
        yyerrpop:
            if (yyssp == yyss)
                goto yyabortlab;
            --yyvsp;
            yystate = *--yyssp;
            continue;
        }
        break;
    }
    if (yyn != 267)
    {
        *++yyvsp = yylval;
        yystate = yyn;
        goto yynewstate;
    }
yyacceptlab:
    if (yyfree_stacks)
    {
        free(yyss);
        free(yyvs);
    }
    return 0;
}

unsigned int __cdecl LowerCase(unsigned int stringValue)
{
    return SL_ConvertToLowercase(gInst, stringValue, g_parse_user, 14);
}

void __cdecl _yy_memcpy(char *to, char *from, unsigned int count)
{
    int v3; // eax

    while ( 1 )
    {
        v3 = count--;
        if ( v3 <= 0 )
            break;
        *to++ = *from++;
    }
}

int __cdecl yylex()
{
    int next_buffer; // [esp+0h] [ebp-24h]
    int yy_next_state; // [esp+8h] [ebp-1Ch]
    int yy_amount_of_matched_text; // [esp+Ch] [ebp-18h]
    unsigned __int8 yy_c; // [esp+13h] [ebp-11h]
    char *yy_bp; // [esp+14h] [ebp-10h]
    int yy_current_state; // [esp+18h] [ebp-Ch]
    int yy_act; // [esp+1Ch] [ebp-8h]
    char *yy_cp; // [esp+20h] [ebp-4h]

    if ( yy_init )
    {
        yy_init = 0;
        if ( !yy_start )
            yy_start = 1;
        if ( !yyin )
            yyin = stdin;
        if ( !yyout )
            yyout = stdout;
        if ( !yy_current_buffer )
            yy_current_buffer = yy_create_buffer(yyin, 0x4000u);
        yy_load_buffer_state();
    }
    while ( 1 )
    {
LABEL_11:
        yy_cp = yy_c_buf_p;
        *yy_c_buf_p = yy_hold_char;
        yy_bp = yy_cp;
        yy_current_state = yy_start;
        do
        {
yy_match:
            yy_c = yy_ec[(unsigned __int8)*yy_cp];
            if ( yy_accept[yy_current_state] )
            {
                yy_last_accepting_state = yy_current_state;
                yy_last_accepting_cpos = yy_cp;
            }
            while ( yy_chk[yy_c + yy_base[yy_current_state]] != yy_current_state )
            {
                yy_current_state = yy_def[yy_current_state];
                if ( yy_current_state >= 262 )
                    yy_c = yy_meta[yy_c];
            }
            yy_current_state = yy_nxt[yy_c + yy_base[yy_current_state]];
            ++yy_cp;
        }
        while ( yy_base[yy_current_state] != 443 );
        while ( 2 )
        {
            yy_act = yy_accept[yy_current_state];
            if ( !yy_accept[yy_current_state] )
            {
                yy_cp = yy_last_accepting_cpos;
                yy_act = yy_accept[yy_last_accepting_state];
            }
            yytext = yy_bp;
            yyleng = yy_cp - yy_bp;
            yy_hold_char = *yy_cp;
            *yy_cp = 0;
            yy_c_buf_p = yy_cp;
do_action:
            switch ( yy_act )
            {
                case 0:
                    *yy_cp = yy_hold_char;
                    yy_cp = yy_last_accepting_cpos;
                    yy_current_state = yy_last_accepting_state;
                    continue;
                case 1:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    goto LABEL_11;
                case 2:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    yy_start = 3;
                    goto LABEL_11;
                case 3:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    goto LABEL_11;
                case 4:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    goto LABEL_11;
                case 5:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    goto LABEL_11;
                case 6:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    yy_start = 5;
                    goto LABEL_11;
                case 7:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return StringValue(yytext + 1, yyleng - 2) != 0 ? 259 : 257;
                case 8:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return StringValue(yytext + 2, yyleng - 3) != 0 ? 260 : 257;
                case 9:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return HashValue(yytext + 2, yyleng - 3) != 0 ? 261 : 257;
                case 10:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 262;
                case 11:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 263;
                case 12:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 264;
                case 13:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 265;
                case 14:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 266;
                case 15:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 267;
                case 16:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 268;
                case 17:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 269;
                case 18:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 270;
                case 19:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 271;
                case 20:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 272;
                case 21:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 273;
                case 22:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 274;
                case 23:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 275;
                case 24:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 276;
                case 25:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 277;
                case 26:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 278;
                case 27:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 279;
                case 28:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 280;
                case 29:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 281;
                case 30:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 282;
                case 31:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 283;
                case 32:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 284;
                case 33:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 285;
                case 34:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 286;
                case 35:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 287;
                case 36:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    IntegerValue(yytext);
                    return 288;
                case 37:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    FloatValue(yytext);
                    return 289;
                case 38:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 291;
                case 39:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 290;
                case 40:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 295;
                case 41:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 292;
                case 42:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 294;
                case 43:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 293;
                case 44:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 296;
                case 45:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 297;
                case 46:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 298;
                case 47:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 299;
                case 48:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 300;
                case 49:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 301;
                case 50:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 302;
                case 51:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 303;
                case 52:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 304;
                case 53:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 305;
                case 54:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 306;
                case 55:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 307;
                case 56:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 308;
                case 57:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 309;
                case 58:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 310;
                case 59:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 311;
                case 60:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 312;
                case 61:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 313;
                case 62:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 314;
                case 63:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 315;
                case 64:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 316;
                case 65:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 317;
                case 66:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 318;
                case 67:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 319;
                case 68:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 320;
                case 69:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 321;
                case 70:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 322;
                case 71:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 323;
                case 72:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 324;
                case 73:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 326;
                case 74:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 327;
                case 75:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 328;
                case 76:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 329;
                case 77:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 330;
                case 78:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 331;
                case 79:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 332;
                case 80:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 333;
                case 81:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 334;
                case 82:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 335;
                case 83:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 336;
                case 84:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 337;
                case 85:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 338;
                case 86:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 339;
                case 87:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 340;
                case 88:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 341;
                case 89:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 342;
                case 90:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 343;
                case 91:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    return 344;
                case 92:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    TextValue(yytext, yyleng);
                    return 258;
                case 93:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    TextValue(yytext, yyleng);
                    return 325;
                case 94:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    CompileError(gInst, g_sourcePos, "bad token '%s'", yytext);
                    return 257;
                case 95:
                    yylval.pos = g_out_pos;
                    g_sourcePos = g_out_pos;
                    g_out_pos += yyleng;
                    fwrite(yytext, yyleng, 1u, yyout);
                    goto LABEL_11;
                case 96:
                    yy_amount_of_matched_text = yy_cp - yytext - 1;
                    *yy_cp = yy_hold_char;
                    if ( !yy_current_buffer->yy_buffer_status )
                    {
                        yy_n_chars = yy_current_buffer->yy_n_chars;
                        yy_current_buffer->yy_input_file = yyin;
                        yy_current_buffer->yy_buffer_status = 1;
                    }
                    if ( yy_c_buf_p <= &yy_current_buffer->yy_ch_buf[yy_n_chars] )
                    {
                        yy_c_buf_p = &yytext[yy_amount_of_matched_text];
                        yy_current_state = yy_get_previous_state();
                        yy_next_state = yy_try_NUL_trans(yy_current_state);
                        yy_bp = yytext;
                        if ( yy_next_state )
                        {
                            yy_cp = ++yy_c_buf_p;
                            yy_current_state = yy_next_state;
                            goto yy_match;
                        }
                        yy_cp = yy_c_buf_p;
                        continue;
                    }
                    next_buffer = yy_get_next_buffer();
                    if ( !next_buffer )
                    {
                        yy_c_buf_p = &yytext[yy_amount_of_matched_text];
                        yy_current_state = yy_get_previous_state();
                        yy_cp = yy_c_buf_p;
                        yy_bp = yytext;
                        goto yy_match;
                    }
                    if ( next_buffer != 1 )
                    {
                        if ( next_buffer != 2 )
                            goto LABEL_11;
                        yy_c_buf_p = &yy_current_buffer->yy_ch_buf[yy_n_chars];
                        yy_current_state = yy_get_previous_state();
                        yy_cp = yy_c_buf_p;
                        yy_bp = yytext;
                        continue;
                    }
                    yy_did_buffer_switch_on_eof = 0;
                    if ( CL_LocalClient_GetActiveCount() )
                    {
                        yy_c_buf_p = yytext;
                        yy_act = (yy_start - 1) / 2 + 97;
                        goto do_action;
                    }
                    if ( !yy_did_buffer_switch_on_eof )
                        yyrestart(yyin);
                    break;
                case 97:
                case 98:
                case 99:
                    return 0;
                default:
                    yy_fatal_error("fatal flex scanner internal error--no action found");
            }
            break;
        }
    }
}

void __cdecl TextValue(char *str, int len)
{
    yylval.val.stringValue = SL_GetStringOfSize(gInst, str, 0, len + 1, 14);
}

unsigned int HashValue(char *str, int len)
{
    yylval.val.stringValue = Com_HashString(str, len);
    return yylval.val.intValue;
}

int __cdecl StringValue(char *str, int len)
{
    char v3; // [esp+0h] [ebp-200Ch]
    char stra[8192]; // [esp+4h] [ebp-2008h] BYREF
    char *v5; // [esp+2008h] [ebp-4h]
    char *v6; // [esp+2014h] [ebp+8h]
    int v7; // [esp+2018h] [ebp+Ch]

    if ( len < 0x2000 )
    {
        v5 = stra;
        while ( len )
        {
            if ( *str == 92 )
            {
                v7 = len - 1;
                if ( !v7 )
                    break;
                v6 = str + 1;
                v3 = *v6;
                if ( *v6 == 110 )
                {
                    *v5++ = 10;
                }
                else if ( v3 == 114 )
                {
                    *v5++ = 13;
                }
                else
                {
                    if ( v3 == 116 )
                        *v5 = 9;
                    else
                        *v5 = *v6;
                    ++v5;
                }
                len = v7 - 1;
                str = v6 + 1;
            }
            else
            {
                --len;
                *v5++ = *str++;
            }
        }
        *v5 = 0;
        yylval.val.stringValue = SL_GetString_(gInst, stra, g_parse_user, 14);
        return 1;
    }
    else
    {
        CompileError(gInst, g_sourcePos, "max string length exceeded: \"%s\"", str);
        return 0;
    }
}

void __cdecl IntegerValue(char *str)
{
    sscanf(str, "%d", &yylval);
}

void __cdecl FloatValue(char *str)
{
    sscanf(str, "%f", &yylval);
}

int __cdecl yy_get_next_buffer()
{
    int yy_c_buf_p_offset; // [esp+4h] [ebp-20h]
    yy_buffer_state *b; // [esp+8h] [ebp-1Ch]
    signed int num_to_read; // [esp+Ch] [ebp-18h]
    char *source; // [esp+10h] [ebp-14h]
    int ret_val; // [esp+14h] [ebp-10h]
    char *dest; // [esp+18h] [ebp-Ch]
    int number_to_move; // [esp+1Ch] [ebp-8h]
    int i; // [esp+20h] [ebp-4h]

    dest = yy_current_buffer->yy_ch_buf;
    source = yytext;
    if ( yy_c_buf_p > &dest[yy_n_chars + 1] )
        yy_fatal_error("fatal flex scanner internal error--end of buffer missed");
    if ( yy_current_buffer->yy_fill_buffer )
    {
        number_to_move = yy_c_buf_p - yytext - 1;
        for ( i = 0; i < number_to_move; ++i )
            *dest++ = *source++;
        if ( yy_current_buffer->yy_buffer_status == 2 )
        {
            yy_n_chars = 0;
            yy_current_buffer->yy_n_chars = 0;
        }
        else
        {
            for ( num_to_read = yy_current_buffer->yy_buf_size - number_to_move - 1;
                        num_to_read <= 0;
                        num_to_read = yy_current_buffer->yy_buf_size - number_to_move - 1 )
            {
                b = yy_current_buffer;
                yy_c_buf_p_offset = yy_c_buf_p - yy_current_buffer->yy_ch_buf;
                if ( yy_current_buffer->yy_is_our_buffer )
                {
                    if ( (signed int)(2 * yy_current_buffer->yy_buf_size) > 0 )
                        yy_current_buffer->yy_buf_size *= 2;
                    else
                        yy_current_buffer->yy_buf_size += yy_current_buffer->yy_buf_size >> 3;
                    b->yy_ch_buf = (char *)yy_flex_realloc(b->yy_ch_buf, b->yy_buf_size + 2);
                }
                else
                {
                    yy_current_buffer->yy_ch_buf = 0;
                }
                if ( !b->yy_ch_buf )
                    yy_fatal_error("fatal error - scanner input buffer overflow");
                yy_c_buf_p = &b->yy_ch_buf[yy_c_buf_p_offset];
            }
            if ( num_to_read > 0x2000 )
                num_to_read = 0x2000;
            yy_n_chars = Scr_ScanFile(gInst, &yy_current_buffer->yy_ch_buf[number_to_move], num_to_read);
            yy_current_buffer->yy_n_chars = yy_n_chars;
        }
        if ( yy_n_chars )
        {
            ret_val = 0;
        }
        else if ( number_to_move )
        {
            ret_val = 2;
            yy_current_buffer->yy_buffer_status = 2;
        }
        else
        {
            ret_val = 1;
            yyrestart(yyin);
        }
        yy_n_chars += number_to_move;
        yy_current_buffer->yy_ch_buf[yy_n_chars] = 0;
        yy_current_buffer->yy_ch_buf[yy_n_chars + 1] = 0;
        yytext = yy_current_buffer->yy_ch_buf;
        return ret_val;
    }
    else if ( yy_c_buf_p - yytext == 1 )
    {
        return 1;
    }
    else
    {
        return 2;
    }
}

int __cdecl yy_get_previous_state()
{
    int v1; // [esp+0h] [ebp-10h]
    unsigned __int8 yy_c; // [esp+7h] [ebp-9h]
    int yy_current_state; // [esp+8h] [ebp-8h]
    char *yy_cp; // [esp+Ch] [ebp-4h]

    yy_current_state = yy_start;
    for ( yy_cp = yytext; yy_cp < yy_c_buf_p; ++yy_cp )
    {
        if ( *yy_cp )
            v1 = yy_ec[(unsigned __int8)*yy_cp];
        else
            v1 = 1;
        yy_c = v1;
        if ( yy_accept[yy_current_state] )
        {
            yy_last_accepting_state = yy_current_state;
            yy_last_accepting_cpos = yy_cp;
        }
        while ( yy_chk[yy_c + yy_base[yy_current_state]] != yy_current_state )
        {
            yy_current_state = yy_def[yy_current_state];
            if ( yy_current_state >= 262 )
                yy_c = yy_meta[yy_c];
        }
        yy_current_state = yy_nxt[yy_c + yy_base[yy_current_state]];
    }
    return yy_current_state;
}

int __cdecl yy_try_NUL_trans(int yy_current_state)
{
    unsigned __int8 yy_c; // [esp+7h] [ebp-5h]
    int yy_current_statea; // [esp+14h] [ebp+8h]

    yy_c = 1;
    if ( yy_accept[yy_current_state] )
    {
        yy_last_accepting_state = yy_current_state;
        yy_last_accepting_cpos = yy_c_buf_p;
    }
    while ( yy_chk[yy_c + yy_base[yy_current_state]] != yy_current_state )
    {
        yy_current_state = yy_def[yy_current_state];
        if ( yy_current_state >= 262 )
            yy_c = yy_meta[yy_c];
    }
    yy_current_statea = yy_nxt[yy_c + yy_base[yy_current_state]];
    return yy_current_statea != 261 ? yy_current_statea : 0;
}

void __cdecl yyrestart(FILE *input_file)
{
    if ( !yy_current_buffer )
        yy_current_buffer = yy_create_buffer(yyin, 0x4000u);
    yy_init_buffer(yy_current_buffer, input_file);
    yy_load_buffer_state();
}

void __cdecl yy_load_buffer_state()
{
    yy_n_chars = yy_current_buffer->yy_n_chars;
    yy_c_buf_p = yy_current_buffer->yy_buf_pos;
    yytext = yy_c_buf_p;
    yyin = yy_current_buffer->yy_input_file;
    yy_hold_char = *yy_c_buf_p;
}

yy_buffer_state *__cdecl yy_create_buffer(_iobuf *file, unsigned int size)
{
    yy_buffer_state *b; // [esp+0h] [ebp-4h]

    b = (yy_buffer_state *)yy_flex_alloc(0x28u);
    if ( !b )
        yy_fatal_error("out of dynamic memory in yy_create_buffer()");
    b->yy_buf_size = size;
    b->yy_ch_buf = (char *)yy_flex_alloc(b->yy_buf_size + 2);
    if ( !b->yy_ch_buf )
        yy_fatal_error("out of dynamic memory in yy_create_buffer()");
    b->yy_is_our_buffer = 1;
    yy_init_buffer(b, file);
    return b;
}

void __cdecl yy_init_buffer(yy_buffer_state *b, _iobuf *file)
{
    yy_flush_buffer(b);
    b->yy_input_file = file;
    b->yy_fill_buffer = 1;
    b->yy_is_interactive = 0;
}

void __cdecl yy_flush_buffer(yy_buffer_state *b)
{
    if ( b )
    {
        b->yy_n_chars = 0;
        *b->yy_ch_buf = 0;
        b->yy_ch_buf[1] = 0;
        b->yy_buf_pos = b->yy_ch_buf;
        b->yy_at_bol = 1;
        b->yy_buffer_status = 0;
        if ( b == yy_current_buffer )
            yy_load_buffer_state();
    }
}

void __cdecl    yy_fatal_error(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(2);
}

void *__cdecl yy_flex_alloc(unsigned int size)
{
    return malloc(size);
}

void *__cdecl yy_flex_realloc(void *ptr, unsigned int size)
{
    return realloc(ptr, size);
}

int __cdecl yyerror()
{
    if ( yychar )
    {
        if ( yychar != 257 )
            CompileError(gInst, g_sourcePos, "bad syntax");
    }
    else
    {
        CompileError(gInst, g_sourcePos, "unexpected end of file found");
    }
    return 0;
}

void __cdecl ScriptParse(scriptInstance_t inst, sval_u *parseData, unsigned __int8 user)
{
    yy_buffer_state buffer_state; // [esp+0h] [ebp-28h] BYREF

    g_out_pos = -1;
    g_sourcePos = 0;
    g_parse_user = user;
    g_dummyVal.stringValue = 0;
    gInst = inst;
    yy_init = 1;
    buffer_state.yy_buf_size = 0x4000;
    buffer_state.yy_ch_buf = ch_buf;
    buffer_state.yy_is_our_buffer = 0;
    yy_init_buffer(&buffer_state, 0);
    yy_current_buffer = &buffer_state;
    yy_start = 3;
    yyparse();
    *parseData = yaccResult;
}

