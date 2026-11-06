#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <math.h>
#endif

/* =========================================================
 *  LIMITES
 * ========================================================= */
#define MAX_PRODUTOS      100
#define MAX_CLIENTES       50
#define MAX_VENDAS        200
#define MAX_PEDIDOS        50
#define MAX_TENTATIVAS      5
#define ESTOQUE_MINIMO      5

/* =========================================================
 *  CREDENCIAIS
 * ========================================================= */
#define USUARIO  "admin"
#define SENHA    "1234"

/* =========================================================
 *  CORES ANSI
 * ========================================================= */
#define COR_RESET    "\033[0m"
#define COR_VERMELHO "\033[1;31m"
#define COR_VERDE    "\033[1;32m"
#define COR_AMARELO  "\033[1;33m"
#define COR_AZUL     "\033[1;34m"
#define COR_CIANO    "\033[1;36m"
#define COR_BRANCO   "\033[1;37m"
#define COR_CINZA    "\033[0;37m"

/* =========================================================
 *  ESTRUTURAS DE DADOS
 * ========================================================= */

/* Produto do catalogo */
typedef struct {
    char   nome[50];
    double preco;          /* preco de venda */
    double custo;          /* preco de custo (para margem de lucro) */
    double imposto_perc;   /* aliquota de imposto em % */
    int    quantidade;
} Produto;

/* Registro de uma venda realizada */
typedef struct {
    int    id_produto;
    int    id_cliente;      /* -1 se nao identificado */
    int    qtde;
    double subtotal;
    double desconto;
    double imposto_valor;
    double total;
    double valor_pago;
    double troco;
} Venda;

/* Cliente */
typedef struct {
    char   nome[50];
    char   telefone[20];
    int    total_compras;
    double valor_total_gasto;
} Cliente;

/* Pedido de compra (reposicao de estoque) */
typedef struct {
    int    id_produto;
    int    qtde_pedida;
    double preco_unitario;
    int    recebido;        /* 0 = pendente, 1 = recebido */
} Pedido;

/* =========================================================
 *  VARIAVEIS GLOBAIS
 * ========================================================= */
Produto produtos[MAX_PRODUTOS];
int     total_produtos = 0;

Cliente clientes[MAX_CLIENTES];
int     total_clientes = 0;

Venda   vendas[MAX_VENDAS];
int     total_vendas = 0;

Pedido  pedidos[MAX_PEDIDOS];
int     total_pedidos = 0;

double  caixa_total = 0.0;   /* fluxo de caixa acumulado */

/* =========================================================
 *  PROTOTIPOS
 * ========================================================= */
void limpar_buffer(void);
void limpar_tela(void);
void pausar(void);
void linha(int tamanho, const char *cor);
void linha_simples(int tamanho, const char *cor);

void tela_login(void);
int  menu_principal(void);
void tela_saida(void);

/* modulo estoque */
void registrar_produtos(void);
void exibir_tabela_estoque(void);
void ver_estoque(void);
void inventario(void);

/* modulo vendas */
void realizar_venda(void);

/* modulo clientes */
void menu_clientes(void);
void cadastrar_cliente(void);
void listar_clientes(void);
void historico_cliente(void);

/* modulo relatorios */
void menu_relatorios(void);
void dashboard(void);
void relatorio_desempenho(void);
void relatorio_margem(void);
void fluxo_caixa(void);

#ifdef _WIN32
/* dashboard grafico (janela GDI) */
void dashboard_grafico(void);
#endif

/* modulo compras */
void menu_compras(void);
void novo_pedido_compra(void);
void listar_pedidos(void);
void receber_pedido(void);

/* =========================================================
 *  AUXILIARES
 * ========================================================= */
void limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void limpar_tela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar(void) {
    printf("\n%s Pressione ENTER para continuar...%s", COR_CINZA, COR_RESET);
    limpar_buffer();
}

void linha(int tamanho, const char *cor) {
    int i;
    printf("%s", cor);
    for (i = 0; i < tamanho; i++) printf("=");
    printf("%s\n", COR_RESET);
}

void linha_simples(int tamanho, const char *cor) {
    int i;
    printf("%s", cor);
    for (i = 0; i < tamanho; i++) printf("-");
    printf("%s\n", COR_RESET);
}

/* =========================================================
 *  LOGIN
 * ========================================================= */
void tela_login(void) {
    char usuario_input[30];
    char senha_input[30];
    int  tentativas  = 0;
    int  autenticado = 0;

    while (tentativas < MAX_TENTATIVAS && !autenticado) {
        limpar_tela();

        linha(76, COR_AZUL);
        printf("%s", COR_VERMELHO);
        printf("   ######   #######  ##     ## ######## ########   ######  ####    ###    ##\n");
        printf("  ##    ## ##     ## ###   ### ##       ##     ## ##    ##  ##    ## ##   ##\n");
        printf("  ##       ##     ## #### #### ##       ##     ## ##        ##   ##   ##  ##\n");
        printf("  ##       ##     ## ## ### ## ######   ########  ##        ##  ##     ## ##\n");
        printf("  ##       ##     ## ##     ## ##       ##   ##   ##        ##  ######### ##\n");
        printf("  ##    ## ##     ## ##     ## ##       ##    ##  ##    ##  ##  ##     ## ##\n");
        printf("   ######   #######  ##     ## ######## ##     ##  ######  #### ##     ## ########\n");
        printf("\n");
        printf("   ######     ###    ##    ## ########  #######   ######\n");
        printf("  ##    ##   ## ##   ###   ##    ##    ##     ## ##    ##\n");
        printf("  ##        ##   ##  ####  ##    ##    ##     ## ##\n");
        printf("   ######  ##     ## ## ## ##    ##    ##     ##  ######\n");
        printf("        ## ######### ##  ####    ##    ##     ##       ##\n");
        printf("  ##    ## ##     ## ##   ###    ##    ##     ## ##    ##\n");
        printf("   ######  ##     ## ##    ##    ##     #######   ######\n");
        printf("%s", COR_RESET);
        linha(76, COR_AZUL);
        printf("%s         S I S T E M A   D E   V E N D A S%s\n", COR_CIANO, COR_RESET);
        linha(76, COR_AZUL);

        printf("\n%s  [ AUTENTICACAO ]%s\n\n", COR_AMARELO, COR_RESET);

        if (tentativas > 0) {
            printf("  %s! Credenciais invalidas. Tentativa %d/%d%s\n\n",
                   COR_VERMELHO, tentativas, MAX_TENTATIVAS, COR_RESET);
        }

        printf("  %sUsuario:%s ", COR_CIANO, COR_RESET);
        fgets(usuario_input, sizeof(usuario_input), stdin);
        usuario_input[strcspn(usuario_input, "\n")] = '\0';

        printf("  %sSenha:  %s ", COR_CIANO, COR_RESET);
        fgets(senha_input, sizeof(senha_input), stdin);
        senha_input[strcspn(senha_input, "\n")] = '\0';

        if (strcmp(usuario_input, USUARIO) == 0 &&
            strcmp(senha_input,   SENHA)   == 0) {
            autenticado = 1;
        } else {
            tentativas++;
        }
    }

    if (!autenticado) {
        limpar_tela();
        linha(54, COR_VERMELHO);
        printf("\n  %s ACESSO BLOQUEADO!%s\n", COR_VERMELHO, COR_RESET);
        printf("  Numero maximo de tentativas (%d) excedido.\n", MAX_TENTATIVAS);
        printf("  O programa sera encerrado.\n\n");
        linha(54, COR_VERMELHO);
        exit(1);
    }

    limpar_tela();
    printf("\n  %s Bem-vindo, %s!%s\n", COR_VERDE, USUARIO, COR_RESET);
    printf("  %sLogin realizado com sucesso.%s\n", COR_CINZA, COR_RESET);
    pausar();
}

/* =========================================================
 *  MENU PRINCIPAL
 * ========================================================= */
int menu_principal(void) {
    int opcao;
    limpar_tela();

    linha(54, COR_AZUL);
    printf("%s   *** SISTEMA DE VENDAS ***%s\n", COR_BRANCO, COR_RESET);
    linha(54, COR_AZUL);

    printf("\n  %s[1]%s  Registro de Produtos\n",    COR_CIANO,    COR_RESET);
    printf("  %s[2]%s  Estoque de Produtos\n",        COR_CIANO,    COR_RESET);
    printf("  %s[3]%s  Realizar Venda\n",              COR_CIANO,    COR_RESET);
    printf("  %s[4]%s  Clientes e CRM\n",              COR_CIANO,    COR_RESET);
    printf("  %s[5]%s  Relatorios e Dashboard\n",      COR_CIANO,    COR_RESET);
    printf("  %s[6]%s  Compras e Suprimentos\n",       COR_CIANO,    COR_RESET);
    printf("  %s[0]%s  Sair do Programa\n\n",          COR_VERMELHO, COR_RESET);
    linha(54, COR_AZUL);

    printf("\n  %sEscolha uma opcao: %s", COR_AMARELO, COR_RESET);
    if (scanf("%d", &opcao) != 1) opcao = -1;
    limpar_buffer();

    return opcao;
}

/* =========================================================
 *  1. REGISTRO DE PRODUTOS
 * ========================================================= */
void registrar_produtos(void) {
    char     resposta;
    Produto *p;

    limpar_tela();
    linha(54, COR_VERDE);
    printf("%s   [ REGISTRO DE PRODUTOS ]%s\n", COR_VERDE, COR_RESET);
    linha(54, COR_VERDE);

    do {
        if (total_produtos >= MAX_PRODUTOS) {
            printf("\n  %sCapacidade maxima atingida (%d).%s\n",
                   COR_VERMELHO, MAX_PRODUTOS, COR_RESET);
            break;
        }

        p = &produtos[total_produtos];
        p->imposto_perc = 0.0;
        p->custo        = 0.0;

        printf("\n  %sProduto #%d%s\n", COR_AMARELO, total_produtos + 1, COR_RESET);
        linha_simples(44, COR_CINZA);

        printf("  Nome do produto  : ");
        fgets(p->nome, sizeof(p->nome), stdin);
        p->nome[strcspn(p->nome, "\n")] = '\0';

        printf("  Preco de custo   : R$ ");
        while (scanf("%lf", &p->custo) != 1 || p->custo < 0) {
            printf("  %sValor invalido: R$ %s", COR_VERMELHO, COR_RESET);
            limpar_buffer();
        }
        limpar_buffer();

        printf("  Preco de venda   : R$ ");
        while (scanf("%lf", &p->preco) != 1 || p->preco < 0) {
            printf("  %sValor invalido: R$ %s", COR_VERMELHO, COR_RESET);
            limpar_buffer();
        }
        limpar_buffer();

        printf("  Imposto (%%)      : ");
        while (scanf("%lf", &p->imposto_perc) != 1 || p->imposto_perc < 0 || p->imposto_perc > 100) {
            printf("  %sValor invalido (0-100): %s", COR_VERMELHO, COR_RESET);
            limpar_buffer();
        }
        limpar_buffer();

        printf("  Quantidade       : ");
        while (scanf("%d", &p->quantidade) != 1 || p->quantidade < 0) {
            printf("  %sValor invalido: %s", COR_VERMELHO, COR_RESET);
            limpar_buffer();
        }
        limpar_buffer();

        total_produtos++;
        printf("  %s Produto cadastrado!%s\n", COR_VERDE, COR_RESET);

        printf("\n  %sDeseja adicionar mais? (s/n): %s", COR_CIANO, COR_RESET);
        scanf(" %c", &resposta);
        limpar_buffer();
        printf("\n");

    } while (resposta == 's' || resposta == 'S');

    printf("  %sTotal cadastrado: %d produto(s)%s\n", COR_BRANCO, total_produtos, COR_RESET);
    pausar();
}

/* =========================================================
 *  TABELA DE ESTOQUE 
 * ========================================================= */
void exibir_tabela_estoque(void) {
    int      i;
    Produto *p;

    printf("\n  %-4s %-22s %10s %10s %8s %8s\n",
           "#", "NOME", "CUSTO(R$)", "VENDA(R$)", "IMP.%", "QTDE");
    linha_simples(68, COR_CINZA);

    for (i = 0; i < total_produtos; i++) {
        p = &produtos[i];

        if (p->quantidade == 0) {
            printf("  %s%-4d %-22s %10.2f %10.2f %7.1f%% %7d  [SEM ESTOQUE]%s\n",
                   COR_VERMELHO, i+1, p->nome, p->custo, p->preco,
                   p->imposto_perc, p->quantidade, COR_RESET);
        } else if (p->quantidade <= ESTOQUE_MINIMO) {
            printf("  %s%-4d %-22s %10.2f %10.2f %7.1f%% %7d  [ESTOQUE BAIXO]%s\n",
                   COR_AMARELO, i+1, p->nome, p->custo, p->preco,
                   p->imposto_perc, p->quantidade, COR_RESET);
        } else {
            printf("  %s%-4d %-22s %10.2f %10.2f %7.1f%% %7d%s\n",
                   COR_VERDE, i+1, p->nome, p->custo, p->preco,
                   p->imposto_perc, p->quantidade, COR_RESET);
        }
    }
    linha_simples(68, COR_CINZA);
}

/* =========================================================
 *  INVENTARIO — valor total do estoque
 * ========================================================= */
void inventario(void) {
    int    i;
    double total_custo  = 0.0;
    double total_venda  = 0.0;
    int    itens_zerados = 0;
    int    itens_baixos  = 0;

    limpar_tela();
    linha(60, COR_CIANO);
    printf("%s   [ INVENTARIO DO ESTOQUE ]%s\n", COR_CIANO, COR_RESET);
    linha(60, COR_CIANO);

    if (total_produtos == 0) {
        printf("\n  %sNenhum produto cadastrado.%s\n", COR_AMARELO, COR_RESET);
        pausar();
        return;
    }

    printf("\n  %-22s %10s %10s %12s\n", "PRODUTO", "QTDE", "VL.CUSTO", "VL.VENDA");
    linha_simples(58, COR_CINZA);

    for (i = 0; i < total_produtos; i++) {
        double vc = produtos[i].custo * produtos[i].quantidade;
        double vv = produtos[i].preco * produtos[i].quantidade;
        total_custo += vc;
        total_venda += vv;
        if (produtos[i].quantidade == 0)           itens_zerados++;
        else if (produtos[i].quantidade <= ESTOQUE_MINIMO) itens_baixos++;

        printf("  %-22s %10d %10.2f %12.2f\n",
               produtos[i].nome, produtos[i].quantidade, vc, vv);
    }

    linha_simples(58, COR_CINZA);
    printf("\n  %sValor total (custo) : %sR$ %.2f%s\n",  COR_BRANCO, COR_VERDE,   total_custo, COR_RESET);
    printf("  %sValor total (venda) : %sR$ %.2f%s\n",  COR_BRANCO, COR_VERDE,   total_venda, COR_RESET);
    printf("  %sLucro potencial     : %sR$ %.2f%s\n",  COR_BRANCO, COR_AMARELO, total_venda - total_custo, COR_RESET);
    printf("\n  %sItens zerados       : %s%d%s\n",      COR_BRANCO, COR_VERMELHO, itens_zerados, COR_RESET);
    printf("  %sItens com estoque baixo: %s%d%s\n",    COR_BRANCO, COR_AMARELO,  itens_baixos,  COR_RESET);

    pausar();
}

/* =========================================================
 *  2. MENU DE ESTOQUE
 * ========================================================= */
void ver_estoque(void) {
    int      opcao_est;
    int      idx;
    double   novo_preco;
    Produto *p;

    do {
        limpar_tela();
        linha(68, COR_CIANO);
        printf("%s   [ ESTOQUE DE PRODUTOS ]%s\n", COR_CIANO, COR_RESET);
        linha(68, COR_CIANO);

        if (total_produtos == 0) {
            printf("\n  %sNenhum produto cadastrado.%s\n", COR_AMARELO, COR_RESET);
            pausar();
            return;
        }

        exibir_tabela_estoque();

        printf("\n  %sLegenda:%s\n", COR_BRANCO, COR_RESET);
        printf("  %s[VERMELHO]%s = Sem estoque\n",       COR_VERMELHO, COR_RESET);
        printf("  %s[AMARELO] %s = Estoque baixo (<= %d)\n", COR_AMARELO, COR_RESET, ESTOQUE_MINIMO);
        printf("  %s[VERDE]   %s = Disponivel\n",        COR_VERDE,    COR_RESET);

        printf("\n");
        linha_simples(68, COR_CIANO);
        printf("  %s[1]%s  Alterar preco de venda\n",  COR_AMARELO, COR_RESET);
        printf("  %s[2]%s  Ver Inventario\n",           COR_AMARELO, COR_RESET);
        printf("  %s[0]%s  Voltar\n",                   COR_CINZA,   COR_RESET);
        linha_simples(68, COR_CIANO);
        printf("\n  %sEscolha: %s", COR_AMARELO, COR_RESET);

        if (scanf("%d", &opcao_est) != 1) opcao_est = -1;
        limpar_buffer();

        if (opcao_est == 1) {
            limpar_tela();
            linha(68, COR_CIANO);
            printf("%s   [ ALTERAR PRECO ]%s\n", COR_AMARELO, COR_RESET);
            linha(68, COR_CIANO);

            exibir_tabela_estoque();

            printf("\n  %sNumero do produto: %s", COR_CIANO, COR_RESET);
            while (scanf("%d", &idx) != 1 || idx < 1 || idx > total_produtos) {
                printf("  %sInvalido: %s", COR_VERMELHO, COR_RESET);
                limpar_buffer();
            }
            limpar_buffer();

            p = &produtos[idx - 1];
            printf("  %sProduto: %s%s%s\n", COR_CINZA, COR_BRANCO, p->nome, COR_RESET);
            printf("  %sPreco atual: %sR$ %.2f%s\n", COR_CINZA, COR_VERDE, p->preco, COR_RESET);

            printf("  %sNovo preco: R$ %s", COR_CIANO, COR_RESET);
            while (scanf("%lf", &novo_preco) != 1 || novo_preco < 0) {
                printf("  %sInvalido: R$ %s", COR_VERMELHO, COR_RESET);
                limpar_buffer();
            }
            limpar_buffer();

            p->preco = novo_preco;
            printf("\n  %s Preco atualizado! %s agora custa R$ %.2f%s\n",
                   COR_VERDE, p->nome, p->preco, COR_RESET);
            pausar();

        } else if (opcao_est == 2) {
            inventario();
        }

    } while (opcao_est != 0);
}

/* =========================================================
 *  3. REALIZAR VENDA
 * ========================================================= */
void realizar_venda(void) {
    int         i;
    int         idx;
    int         idx_cliente;
    int         qtde_venda;
    double      subtotal;
    double      desconto_perc;
    double      valor_desconto;
    double      imposto_valor;
    double      total_com_desc;
    double      valor_pago;
    double      troco;
    char        usa_cliente;
    Produto    *p;
    Cliente    *cl;
    Venda      *v;
    const char *cor;

    limpar_tela();
    linha(60, COR_AMARELO);
    printf("%s   [ REALIZAR VENDA ]%s\n", COR_AMARELO, COR_RESET);
    linha(60, COR_AMARELO);

    if (total_produtos == 0) {
        printf("\n  %sNenhum produto cadastrado.%s\n", COR_VERMELHO, COR_RESET);
        pausar();
        return;
    }

    /* Lista produtos */
    printf("\n  %-4s %-22s %12s %10s\n", "#", "NOME", "PRECO (R$)", "QTDE");
    linha_simples(55, COR_CINZA);
    for (i = 0; i < total_produtos; i++) {
        p   = &produtos[i];
        cor = (p->quantidade == 0) ? COR_VERMELHO : COR_VERDE;
        printf("  %s%-4d %-22s %12.2f %10d%s\n",
               cor, i+1, p->nome, p->preco, p->quantidade, COR_RESET);
    }
    linha_simples(55, COR_CINZA);

    idx = 0;
    printf("\n  %sSelecione o produto: %s", COR_CIANO, COR_RESET);
    while (scanf("%d", &idx) != 1 || idx < 1 || idx > total_produtos) {
        printf("  %sInvalido: %s", COR_VERMELHO, COR_RESET);
        limpar_buffer();
    }
    limpar_buffer();

    p = &produtos[idx - 1];

    if (p->quantidade == 0) {
        printf("\n  %sProduto sem estoque!%s\n", COR_VERMELHO, COR_RESET);
        pausar();
        return;
    }

    /* Quantidade */
    qtde_venda = 0;
    printf("  %sQuantidade (disponivel: %d): %s", COR_CIANO, p->quantidade, COR_RESET);
    while (scanf("%d", &qtde_venda) != 1 || qtde_venda <= 0 || qtde_venda > p->quantidade) {
        printf("  %sInvalido: %s", COR_VERMELHO, COR_RESET);
        limpar_buffer();
    }
    limpar_buffer();

    subtotal = p->preco * qtde_venda;

    /* Imposto */
    imposto_valor = subtotal * (p->imposto_perc / 100.0);

    /* Desconto */
    desconto_perc = 0.0;
    printf("  %sDesconto %% (0 = nenhum): %s", COR_CIANO, COR_RESET);
    while (scanf("%lf", &desconto_perc) != 1 || desconto_perc < 0 || desconto_perc > 100) {
        printf("  %sInvalido (0-100): %s", COR_VERMELHO, COR_RESET);
        limpar_buffer();
    }
    limpar_buffer();

    valor_desconto = subtotal * (desconto_perc / 100.0);
    total_com_desc = subtotal - valor_desconto + imposto_valor;

    /* Cliente identificado? */
    idx_cliente = -1;
    if (total_clientes > 0) {
        printf("  %sVincular a cliente? (s/n): %s", COR_CIANO, COR_RESET);
        scanf(" %c", &usa_cliente);
        limpar_buffer();

        if (usa_cliente == 's' || usa_cliente == 'S') {
            printf("\n  %sClientes cadastrados:%s\n", COR_BRANCO, COR_RESET);
            for (i = 0; i < total_clientes; i++) {
                printf("  [%d] %s\n", i+1, clientes[i].nome);
            }
            printf("  %sNumero do cliente: %s", COR_CIANO, COR_RESET);
            while (scanf("%d", &idx_cliente) != 1 || idx_cliente < 1 || idx_cliente > total_clientes) {
                printf("  %sInvalido: %s", COR_VERMELHO, COR_RESET);
                limpar_buffer();
            }
            limpar_buffer();
            idx_cliente--;
        }
    }

    /* Valor pago */
    valor_pago = 0.0;
    printf("  %sValor pago: R$ %s", COR_CIANO, COR_RESET);
    while (scanf("%lf", &valor_pago) != 1 || valor_pago < 0) {
        printf("  %sInvalido: R$ %s", COR_VERMELHO, COR_RESET);
        limpar_buffer();
    }
    limpar_buffer();

    if (valor_pago < total_com_desc) {
        printf("\n  %sValor insuficiente! Total: R$ %.2f | Pago: R$ %.2f%s\n",
               COR_VERMELHO, total_com_desc, valor_pago, COR_RESET);
        pausar();
        return;
    }

    troco = valor_pago - total_com_desc;
    p->quantidade -= qtde_venda;
    caixa_total   += total_com_desc;

    /* Registrar venda */
    if (total_vendas < MAX_VENDAS) {
        v = &vendas[total_vendas];
        v->id_produto   = idx - 1;
        v->id_cliente   = idx_cliente;
        v->qtde         = qtde_venda;
        v->subtotal     = subtotal;
        v->desconto     = valor_desconto;
        v->imposto_valor= imposto_valor;
        v->total        = total_com_desc;
        v->valor_pago   = valor_pago;
        v->troco        = troco;
        total_vendas++;
    }

    /* Atualizar historico do cliente */
    if (idx_cliente >= 0) {
        cl = &clientes[idx_cliente];
        cl->total_compras++;
        cl->valor_total_gasto += total_com_desc;
    }

    /* Cupom */
    limpar_tela();
    linha(52, COR_VERDE);
    printf("%s         CUPOM DE VENDA  #%04d%s\n", COR_VERDE, total_vendas, COR_RESET);
    linha(52, COR_VERDE);

    printf("\n  %sProduto  :%s %s\n",          COR_BRANCO, COR_RESET, p->nome);
    if (idx_cliente >= 0)
        printf("  %sCliente  :%s %s\n",        COR_BRANCO, COR_RESET, clientes[idx_cliente].nome);
    printf("  %sQtd.     :%s %d unidade(s)\n", COR_BRANCO, COR_RESET, qtde_venda);
    printf("  %sPreco Un.:%s R$ %.2f\n",       COR_BRANCO, COR_RESET, p->preco);
    linha_simples(40, COR_CINZA);
    printf("  %sSubtotal :%s R$ %.2f\n",       COR_BRANCO, COR_RESET, subtotal);

    if (desconto_perc > 0)
        printf("  %sDesconto :%s %.1f%% (-R$ %.2f)\n",
               COR_AMARELO, COR_RESET, desconto_perc, valor_desconto);

    if (p->imposto_perc > 0)
        printf("  %sImposto  :%s %.1f%% (+R$ %.2f)\n",
               COR_CINZA, COR_RESET, p->imposto_perc, imposto_valor);

    printf("  %sTOTAL    :%s R$ %.2f\n",       COR_VERDE,  COR_RESET, total_com_desc);
    linha_simples(40, COR_CINZA);
    printf("  %sPago     :%s R$ %.2f\n",        COR_BRANCO, COR_RESET, valor_pago);
    if (troco > 0.001)
        printf("  %sTROCO    :%s R$ %.2f\n",    COR_CIANO,  COR_RESET, troco);
    else
        printf("  %sTROCO    :%s R$ 0.00\n",    COR_CIANO,  COR_RESET);

    linha(52, COR_VERDE);
    printf("  %s Venda #%04d concluida!%s\n\n", COR_VERDE, total_vendas, COR_RESET);

    if (p->quantidade == 0)
        printf("  %s ATENCAO: estoque zerado!%s\n", COR_VERMELHO, COR_RESET);
    else if (p->quantidade <= ESTOQUE_MINIMO)
        printf("  %s AVISO: estoque baixo (%d restantes).%s\n",
               COR_AMARELO, p->quantidade, COR_RESET);

    pausar();
}

/* =========================================================
 *  4. CLIENTES E CRM
 * ========================================================= */
void cadastrar_cliente(void) {
    Cliente *c;

    if (total_clientes >= MAX_CLIENTES) {
        printf("\n  %sCapacidade maxima de clientes atingida.%s\n", COR_VERMELHO, COR_RESET);
        pausar();
        return;
    }

    c = &clientes[total_clientes];
    c->total_compras     = 0;
    c->valor_total_gasto = 0.0;

    limpar_tela();
    linha(54, COR_VERDE);
    printf("%s   [ CADASTRO DE CLIENTE ]%s\n", COR_VERDE, COR_RESET);
    linha(54, COR_VERDE);

    printf("\n  %sNome do cliente : %s", COR_CIANO, COR_RESET);
    fgets(c->nome, sizeof(c->nome), stdin);
    c->nome[strcspn(c->nome, "\n")] = '\0';

    printf("  %sTelefone        : %s", COR_CIANO, COR_RESET);
    fgets(c->telefone, sizeof(c->telefone), stdin);
    c->telefone[strcspn(c->telefone, "\n")] = '\0';

    total_clientes++;
    printf("\n  %s Cliente cadastrado com sucesso!%s\n", COR_VERDE, COR_RESET);
    pausar();
}

void listar_clientes(void) {
    int i;

    limpar_tela();
    linha(60, COR_VERDE);
    printf("%s   [ CLIENTES CADASTRADOS ]%s\n", COR_VERDE, COR_RESET);
    linha(60, COR_VERDE);

    if (total_clientes == 0) {
        printf("\n  %sNenhum cliente cadastrado.%s\n", COR_AMARELO, COR_RESET);
        pausar();
        return;
    }

    printf("\n  %-4s %-24s %-16s %8s %14s\n",
           "#", "NOME", "TELEFONE", "COMPRAS", "TOTAL GASTO");
    linha_simples(72, COR_CINZA);

    for (i = 0; i < total_clientes; i++) {
        printf("  %-4d %-24s %-16s %8d %14.2f\n",
               i+1,
               clientes[i].nome,
               clientes[i].telefone,
               clientes[i].total_compras,
               clientes[i].valor_total_gasto);
    }
    linha_simples(72, COR_CINZA);
    pausar();
}

void historico_cliente(void) {
    int i;
    int idx;
    int encontrou;

    limpar_tela();
    linha(60, COR_VERDE);
    printf("%s   [ HISTORICO DO CLIENTE ]%s\n", COR_VERDE, COR_RESET);
    linha(60, COR_VERDE);

    if (total_clientes == 0 || total_vendas == 0) {
        printf("\n  %sSem dados suficientes.%s\n", COR_AMARELO, COR_RESET);
        pausar();
        return;
    }

    for (i = 0; i < total_clientes; i++)
        printf("  [%d] %s\n", i+1, clientes[i].nome);

    printf("\n  %sSelecione o cliente: %s", COR_CIANO, COR_RESET);
    while (scanf("%d", &idx) != 1 || idx < 1 || idx > total_clientes) {
        printf("  %sInvalido: %s", COR_VERMELHO, COR_RESET);
        limpar_buffer();
    }
    limpar_buffer();
    idx--;

    printf("\n  %sHistorico de compras — %s%s%s\n\n",
           COR_CINZA, COR_BRANCO, clientes[idx].nome, COR_RESET);
    printf("  %-4s %-22s %8s %12s %12s\n",
           "VENDA", "PRODUTO", "QTD", "TOTAL", "PAGO");
    linha_simples(62, COR_CINZA);

    encontrou = 0;
    for (i = 0; i < total_vendas; i++) {
        if (vendas[i].id_cliente == idx) {
            printf("  %-4d %-22s %8d %12.2f %12.2f\n",
                   i+1,
                   produtos[vendas[i].id_produto].nome,
                   vendas[i].qtde,
                   vendas[i].total,
                   vendas[i].valor_pago);
            encontrou = 1;
        }
    }

    if (!encontrou)
        printf("  %sNenhuma compra registrada para este cliente.%s\n",
               COR_AMARELO, COR_RESET);

    linha_simples(62, COR_CINZA);
    printf("  %sTotal gasto: %sR$ %.2f%s  |  Compras: %s%d%s\n",
           COR_BRANCO, COR_VERDE, clientes[idx].valor_total_gasto, COR_RESET,
           COR_VERDE, clientes[idx].total_compras, COR_RESET);
    pausar();
}

void menu_clientes(void) {
    int op;
    do {
        limpar_tela();
        linha(54, COR_VERDE);
        printf("%s   [ CLIENTES E CRM ]%s\n", COR_VERDE, COR_RESET);
        linha(54, COR_VERDE);
        printf("\n  %s[1]%s  Cadastrar Cliente\n",      COR_CIANO, COR_RESET);
        printf("  %s[2]%s  Listar Clientes\n",          COR_CIANO, COR_RESET);
        printf("  %s[3]%s  Historico de Interacoes\n",  COR_CIANO, COR_RESET);
        printf("  %s[0]%s  Voltar\n\n",                 COR_VERMELHO, COR_RESET);
        linha(54, COR_VERDE);
        printf("\n  %sEscolha: %s", COR_AMARELO, COR_RESET);
        if (scanf("%d", &op) != 1) op = -1;
        limpar_buffer();

        switch (op) {
            case 1: cadastrar_cliente();  break;
            case 2: listar_clientes();    break;
            case 3: historico_cliente();  break;
        }
    } while (op != 0);
}

/* =========================================================
 *  5. RELATORIOS E DASHBOARD
 * ========================================================= */

/* ---------------------------------------------------------
 *  DASHBOARD GRAFICO  (somente Windows / GDI)
 * --------------------------------------------------------- */
#ifdef _WIN32

/* Dados compartilhados com o WndProc */
typedef struct {
    double receita;
    double custo_vendido;
    double total_impostos;
    double total_descontos;
    double lucro_bruto;
    double caixa;
    int    n_vendas;
    int    n_clientes;
    int    n_produtos;
    /* produto mais vendido */
    char   prod_nome[50];
    int    prod_qtde;
    /* vendas por produto (para grafico de barras) */
    int    contagem[MAX_PRODUTOS];
} DashData;

static DashData g_dash;

/* Utilitario: desenha texto centralizado em um rect */
static void TextCentrado(HDC hdc, RECT *r, const char *txt) {
    DrawTextA(hdc, txt, -1, r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

/* Desenha um retangulo com borda arredondada colorido e label */
static void CartaoKPI(HDC hdc, int x, int y, int w, int h,
                      COLORREF cor_fundo, COLORREF cor_texto,
                      const char *titulo, const char *valor)
{
    HBRUSH hbr    = CreateSolidBrush(cor_fundo);
    HBRUSH hOld   = (HBRUSH)SelectObject(hdc, hbr);
    HPEN   hpen   = CreatePen(PS_SOLID, 1, RGB(200,200,200));
    HPEN   hOldP  = (HPEN)SelectObject(hdc, hpen);

    RoundRect(hdc, x, y, x+w, y+h, 12, 12);

    SelectObject(hdc, hOld);
    SelectObject(hdc, hOldP);
    DeleteObject(hbr);
    DeleteObject(hpen);

    /* titulo pequeno */
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(120,120,120));
    HFONT hfSmall = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                DEFAULT_PITCH, "Segoe UI");
    HFONT hfOld = (HFONT)SelectObject(hdc, hfSmall);
    RECT rt = { x+8, y+8, x+w-8, y+h/2 };
    DrawTextA(hdc, titulo, -1, &rt, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    /* valor grande */
    SetTextColor(hdc, cor_texto);
    HFONT hfBig = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                               DEFAULT_PITCH, "Segoe UI");
    SelectObject(hdc, hfBig);
    RECT rv = { x+8, y+h/2, x+w-8, y+h-6 };
    DrawTextA(hdc, valor, -1, &rv, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hfOld);
    DeleteObject(hfSmall);
    DeleteObject(hfBig);
}

/* Desenha grafico de barras horizontal para os top-5 produtos */
static void GraficoBarras(HDC hdc, int x, int y, int w, int h)
{
    /* titulo da secao */
    HFONT hfTit = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                               DEFAULT_PITCH, "Segoe UI");
    HFONT hfOld = (HFONT)SelectObject(hdc, hfTit);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(50,50,50));
    RECT rtit = { x, y, x+w, y+24 };
    DrawTextA(hdc, "Vendas por Produto (Top 5)", -1, &rtit,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(hfTit);

    /* encontra os 5 maiores */
    int ordem[5]  = {-1,-1,-1,-1,-1};
    int usados[MAX_PRODUTOS];
    int i, j, k;
    memset(usados, 0, sizeof(usados));

    for (k = 0; k < 5; k++) {
        int melhor = -1;
        for (i = 0; i < total_produtos; i++) {
            if (!usados[i]) {
                if (melhor == -1 || g_dash.contagem[i] > g_dash.contagem[melhor])
                    melhor = i;
            }
        }
        if (melhor == -1) break;
        ordem[k] = melhor;
        usados[melhor] = 1;
    }

    /* max para escala */
    int maxV = 1;
    for (k = 0; k < 5; k++)
        if (ordem[k] >= 0 && g_dash.contagem[ordem[k]] > maxV)
            maxV = g_dash.contagem[ordem[k]];

    COLORREF cores[5] = {
        RGB(41,128,185), RGB(39,174,96), RGB(243,156,18),
        RGB(192,57,43),  RGB(142,68,173)
    };

    HFONT hfLbl = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                               DEFAULT_PITCH, "Segoe UI");
    SelectObject(hdc, hfLbl);

    int bar_area_y = y + 30;
    int bar_h      = (h - 35) / 5;
    if (bar_h < 10) bar_h = 10;
    int label_w    = 130;
    int bar_max_w  = w - label_w - 50;

    for (k = 0; k < 5; k++) {
        if (ordem[k] < 0) continue;
        int idx = ordem[k];
        int qtd = g_dash.contagem[idx];
        if (qtd == 0) continue;

        int by = bar_area_y + k * bar_h;
        int bw = (int)((double)qtd / maxV * bar_max_w);
        if (bw < 2) bw = 2;

        /* nome do produto */
        SetTextColor(hdc, RGB(60,60,60));
        char nome_trunc[22];
        strncpy(nome_trunc, produtos[idx].nome, 21);
        nome_trunc[21] = '\0';
        RECT rl = { x, by + 2, x + label_w - 4, by + bar_h - 2 };
        DrawTextA(hdc, nome_trunc, -1, &rl, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

        /* barra */
        HBRUSH hbr = CreateSolidBrush(cores[k % 5]);
        HBRUSH hOld2 = (HBRUSH)SelectObject(hdc, hbr);
        HPEN hpen = CreatePen(PS_NULL, 0, 0);
        HPEN hOldP = (HPEN)SelectObject(hdc, hpen);
        RoundRect(hdc, x + label_w, by + 3,
                       x + label_w + bw, by + bar_h - 3, 6, 6);
        SelectObject(hdc, hOld2);
        SelectObject(hdc, hOldP);
        DeleteObject(hbr);
        DeleteObject(hpen);

        /* valor */
        char val[16];
        sprintf(val, "%d", qtd);
        SetTextColor(hdc, RGB(60,60,60));
        RECT rv2 = { x + label_w + bw + 4, by + 2,
                     x + label_w + bw + 50, by + bar_h - 2 };
        DrawTextA(hdc, val, -1, &rv2, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    SelectObject(hdc, hfOld);
    DeleteObject(hfLbl);
}

/* Desenha grafico de pizza simples (receita / custo / impostos) */
static void GraficoPizza(HDC hdc, int cx, int cy, int raio)
{
    if (g_dash.receita <= 0.0) return;

    double total = g_dash.receita;
    double partes[3] = {
        g_dash.lucro_bruto   > 0 ? g_dash.lucro_bruto   : 0,
        g_dash.custo_vendido > 0 ? g_dash.custo_vendido : 0,
        g_dash.total_impostos> 0 ? g_dash.total_impostos: 0
    };
    COLORREF cores[3] = { RGB(39,174,96), RGB(243,156,18), RGB(192,57,43) };
    const char *labels[3] = { "Lucro", "Custo", "Impostos" };

    /* titulo */
    HFONT hfTit = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                               DEFAULT_PITCH, "Segoe UI");
    HFONT hfOld = (HFONT)SelectObject(hdc, hfTit);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(50,50,50));
    RECT rtit = { cx - raio, cy - raio - 28, cx + raio, cy - raio - 6 };
    DrawTextA(hdc, "Composicao da Receita", -1, &rtit,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(hfTit);

    HFONT hfLbl = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                               DEFAULT_PITCH, "Segoe UI");
    SelectObject(hdc, hfLbl);

    double angulo = -90.0 * 3.14159265 / 180.0; /* comeÃ§a no topo */
    int k;
    for (k = 0; k < 3; k++) {
        if (partes[k] <= 0) continue;
        double fatia = (partes[k] / total) * 2.0 * 3.14159265;

        /* vertices do setor */
        int x1 = cx + (int)(raio * cos(angulo));
        int y1 = cy + (int)(raio * sin(angulo));
        int x2 = cx + (int)(raio * cos(angulo + fatia));
        int y2 = cy + (int)(raio * sin(angulo + fatia));

        HBRUSH hbr  = CreateSolidBrush(cores[k]);
        HBRUSH hOld2 = (HBRUSH)SelectObject(hdc, hbr);
        HPEN hpen   = CreatePen(PS_SOLID, 1, RGB(255,255,255));
        HPEN hOldP  = (HPEN)SelectObject(hdc, hpen);

        Pie(hdc, cx-raio, cy-raio, cx+raio, cy+raio, x1, y1, x2, y2);

        SelectObject(hdc, hOld2);
        SelectObject(hdc, hOldP);
        DeleteObject(hbr);
        DeleteObject(hpen);

        /* legenda */
        double mid = angulo + fatia / 2.0;
        int lx = cx + (int)((raio + 18) * cos(mid));
        int ly = cy + (int)((raio + 18) * sin(mid));
        char buf[40];
        sprintf(buf, "%s\n%.0f%%", labels[k], (partes[k]/total)*100.0);
        SetTextColor(hdc, RGB(50,50,50));
        RECT rl = { lx - 38, ly - 14, lx + 38, ly + 14 };
        DrawTextA(hdc, buf, -1, &rl, DT_CENTER | DT_VCENTER);

        angulo += fatia;
    }

    SelectObject(hdc, hfOld);
    DeleteObject(hfLbl);
}

/* Procedimento da janela */
static LRESULT CALLBACK DashWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        /* fundo branco */
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH hbrBG = CreateSolidBrush(RGB(245, 247, 250));
        FillRect(hdc, &rc, hbrBG);
        DeleteObject(hbrBG);

        /* ---- Titulo ---- */
        HFONT hfTitle = CreateFontA(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH, "Segoe UI");
        HFONT hfOldT = (HFONT)SelectObject(hdc, hfTitle);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(30, 30, 30));
        RECT rtitle = { 20, 12, rc.right - 20, 50 };
        DrawTextA(hdc, "Dashboard - Painel de Indicadores", -1, &rtitle,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hfOldT);
        DeleteObject(hfTitle);

        /* linha separadora */
        HPEN hpenLine = CreatePen(PS_SOLID, 1, RGB(220,220,220));
        HPEN hOldL = (HPEN)SelectObject(hdc, hpenLine);
        MoveToEx(hdc, 20, 54, NULL);
        LineTo(hdc, rc.right - 20, 54);
        SelectObject(hdc, hOldL);
        DeleteObject(hpenLine);

        /* ---- Cartoes KPI (2 linhas x 4 colunas) ---- */
        char buf[64];
        int card_w = 180, card_h = 68, gap = 12, start_x = 20, start_y = 62;

        /* linha 1 */
        sprintf(buf, "%d", g_dash.n_vendas);
        CartaoKPI(hdc, start_x,                    start_y,
                  card_w, card_h, RGB(255,255,255), RGB(41,128,185),
                  "Total de Vendas", buf);

        sprintf(buf, "R$ %.2f", g_dash.receita);
        CartaoKPI(hdc, start_x + (card_w+gap),     start_y,
                  card_w, card_h, RGB(255,255,255), RGB(39,174,96),
                  "Receita Total", buf);

        sprintf(buf, "R$ %.2f", g_dash.lucro_bruto);
        CartaoKPI(hdc, start_x + 2*(card_w+gap),   start_y,
                  card_w, card_h, RGB(255,255,255),
                  g_dash.lucro_bruto >= 0 ? RGB(39,174,96) : RGB(192,57,43),
                  "Lucro Bruto", buf);

        sprintf(buf, "R$ %.2f", g_dash.caixa);
        CartaoKPI(hdc, start_x + 3*(card_w+gap),   start_y,
                  card_w, card_h, RGB(255,255,255), RGB(41,128,185),
                  "Saldo em Caixa", buf);

        /* linha 2 */
        int sy2 = start_y + card_h + gap;

        sprintf(buf, "R$ %.2f", g_dash.custo_vendido);
        CartaoKPI(hdc, start_x,                    sy2,
                  card_w, card_h, RGB(255,255,255), RGB(243,156,18),
                  "Custo das Vendas", buf);

        sprintf(buf, "R$ %.2f", g_dash.total_impostos);
        CartaoKPI(hdc, start_x + (card_w+gap),     sy2,
                  card_w, card_h, RGB(255,255,255), RGB(192,57,43),
                  "Total de Impostos", buf);

        sprintf(buf, "%d", g_dash.n_clientes);
        CartaoKPI(hdc, start_x + 2*(card_w+gap),   sy2,
                  card_w, card_h, RGB(255,255,255), RGB(142,68,173),
                  "Clientes Cadastrados", buf);

        sprintf(buf, "%d", g_dash.n_produtos);
        CartaoKPI(hdc, start_x + 3*(card_w+gap),   sy2,
                  card_w, card_h, RGB(255,255,255), RGB(52,152,219),
                  "Produtos Cadastrados", buf);

        /* ---- Produto mais vendido ---- */
        if (g_dash.prod_qtde > 0) {
            HFONT hfpm = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                     ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                     CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH, "Segoe UI");
            SelectObject(hdc, hfpm);
            SetTextColor(hdc, RGB(80,80,80));
            char pmsg[120];
            sprintf(pmsg, "Produto mais vendido:  %s  (%d unidades)",
                    g_dash.prod_nome, g_dash.prod_qtde);
            RECT rpm = { 20, sy2 + card_h + 8, rc.right - 20, sy2 + card_h + 28 };
            DrawTextA(hdc, pmsg, -1, &rpm, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(hfpm);
        }

        /* ---- Grafico de barras (esquerda inferior) ---- */
        int graf_y = start_y + 2*(card_h + gap) + 34;
        int graf_h = rc.bottom - graf_y - 20;
        if (graf_h > 40)
            GraficoBarras(hdc, 20, graf_y, 430, graf_h);

        /* ---- Grafico de pizza (direita inferior) ---- */
        int pizza_cx = 20 + 430 + 60 + 80;
        int pizza_cy = graf_y + graf_h / 2 + 10;
        int pizza_r  = (graf_h - 60) / 2;
        if (pizza_r < 20) pizza_r = 20;
        if (pizza_r > 90) pizza_r = 90;
        GraficoPizza(hdc, pizza_cx, pizza_cy, pizza_r);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

/* Ponto de entrada da janela GDI â€” roda em thread separada */
static DWORD WINAPI DashThreadProc(LPVOID param)
{
    (void)param;

    /* inclui math.h apenas aqui para nao poluir o resto */
    HINSTANCE hInst = GetModuleHandleA(NULL);
    const char *CLASSNAME = "SisVendasDash";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = DashWndProc;
    wc.hInstance     = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASSNAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA(CLASSNAME,
                              "Dashboard - Sistema de Vendas",
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              820, 580,
                              NULL, NULL, hInst, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}

void dashboard_grafico(void) {
    /* Preenche estrutura de dados globais com os calculos do dashboard */
    int i;
    memset(&g_dash, 0, sizeof(g_dash));

    for (i = 0; i < total_vendas; i++) {
        g_dash.receita          += vendas[i].total;
        g_dash.total_impostos   += vendas[i].imposto_valor;
        g_dash.total_descontos  += vendas[i].desconto;
        g_dash.custo_vendido    += produtos[vendas[i].id_produto].custo * vendas[i].qtde;
        g_dash.contagem[vendas[i].id_produto] += vendas[i].qtde;
    }
    g_dash.lucro_bruto = g_dash.receita - g_dash.custo_vendido;
    g_dash.caixa       = caixa_total;
    g_dash.n_vendas    = total_vendas;
    g_dash.n_clientes  = total_clientes;
    g_dash.n_produtos  = total_produtos;

    /* produto mais vendido */
    int melhor = -1;
    for (i = 0; i < total_produtos; i++) {
        if (melhor == -1 || g_dash.contagem[i] > g_dash.contagem[melhor])
            melhor = i;
    }
    if (melhor >= 0 && g_dash.contagem[melhor] > 0) {
        strncpy(g_dash.prod_nome, produtos[melhor].nome, 49);
        g_dash.prod_nome[49] = '\0';
        g_dash.prod_qtde = g_dash.contagem[melhor];
    }

    /* Dispara thread com a janela para nao bloquear o console */
    HANDLE hThread = CreateThread(NULL, 0, DashThreadProc, NULL, 0, NULL);
    if (hThread) CloseHandle(hThread);
}
#endif /* _WIN32 */

void dashboard(void) {
    int    i;
    double receita         = 0.0;
    double custo_vendido   = 0.0;
    double total_impostos  = 0.0;
    double total_descontos = 0.0;
    int    prod_mais_idx   = -1;
    int    prod_mais_qtde  = 0;
    int    contagem[MAX_PRODUTOS];

    for (i = 0; i < MAX_PRODUTOS; i++) contagem[i] = 0;

    for (i = 0; i < total_vendas; i++) {
        receita         += vendas[i].total;
        total_impostos  += vendas[i].imposto_valor;
        total_descontos += vendas[i].desconto;
        custo_vendido   += produtos[vendas[i].id_produto].custo * vendas[i].qtde;
        contagem[vendas[i].id_produto] += vendas[i].qtde;
    }

    for (i = 0; i < total_produtos; i++) {
        if (contagem[i] > prod_mais_qtde) {
            prod_mais_qtde = contagem[i];
            prod_mais_idx  = i;
        }
    }

    limpar_tela();
    linha(58, COR_AZUL);
    printf("%s   [ DASHBOARD — PAINEL DE INDICADORES ]%s\n", COR_AZUL, COR_RESET);
    linha(58, COR_AZUL);

    printf("\n  %s+----------------------+------------------+%s\n", COR_CINZA, COR_RESET);
    printf("  %s| INDICADOR            | VALOR            |%s\n", COR_BRANCO, COR_RESET);
    printf("  %s+----------------------+------------------+%s\n", COR_CINZA, COR_RESET);
    printf("  | Total de Vendas      | %s%-16d%s |\n", COR_VERDE,  total_vendas,   COR_RESET);
    printf("  | Receita Total        | %sR$ %-13.2f%s |\n", COR_VERDE,  receita,   COR_RESET);
    printf("  | Custo das Vendas     | %sR$ %-13.2f%s |\n", COR_AMARELO, custo_vendido, COR_RESET);
    printf("  | Lucro Bruto          | %sR$ %-13.2f%s |\n", COR_VERDE,  receita - custo_vendido, COR_RESET);
    printf("  | Total de Impostos    | %sR$ %-13.2f%s |\n", COR_CINZA,  total_impostos,  COR_RESET);
    printf("  | Total de Descontos   | %sR$ %-13.2f%s |\n", COR_AMARELO, total_descontos, COR_RESET);
    printf("  | Saldo em Caixa       | %sR$ %-13.2f%s |\n", COR_VERDE,  caixa_total,   COR_RESET);
    printf("  | Clientes Cadastrados | %s%-16d%s |\n", COR_CIANO,  total_clientes, COR_RESET);
    printf("  | Produtos Cadastrados | %s%-16d%s |\n", COR_CIANO,  total_produtos, COR_RESET);
    printf("  %s+----------------------+------------------+%s\n", COR_CINZA, COR_RESET);

    if (prod_mais_idx >= 0) {
        printf("\n  %sProduto mais vendido:%s %s (%d unidades)\n",
               COR_BRANCO, COR_VERDE,
               produtos[prod_mais_idx].nome, prod_mais_qtde);
    }

#ifdef _WIN32
    printf("\n  %s[Abrindo painel grafico em janela separada...]%s\n",
           COR_CIANO, COR_RESET);
    dashboard_grafico();
#endif

    pausar();
}

void relatorio_desempenho(void) {
    int i;

    limpar_tela();
    linha(68, COR_AZUL);
    printf("%s   [ RELATORIO DE DESEMPENHO DAS VENDAS ]%s\n", COR_AZUL, COR_RESET);
    linha(68, COR_AZUL);

    if (total_vendas == 0) {
        printf("\n  %sNenhuma venda registrada.%s\n", COR_AMARELO, COR_RESET);
        pausar();
        return;
    }

    printf("\n  %-4s %-22s %6s %10s %10s %10s\n",
           "#", "PRODUTO", "QTD", "SUBTOTAL", "DESCONTO", "TOTAL");
    linha_simples(68, COR_CINZA);

    for (i = 0; i < total_vendas; i++) {
        printf("  %-4d %-22s %6d %10.2f %10.2f %10.2f\n",
               i+1,
               produtos[vendas[i].id_produto].nome,
               vendas[i].qtde,
               vendas[i].subtotal,
               vendas[i].desconto,
               vendas[i].total);
    }

    linha_simples(68, COR_CINZA);
    pausar();
}

void relatorio_margem(void) {
    int    i;
    double receita_total = 0.0;
    double custo_total   = 0.0;
    double margem;

    limpar_tela();
    linha(60, COR_AZUL);
    printf("%s   [ RELATORIO DE MARGEM DE LUCRO ]%s\n", COR_AZUL, COR_RESET);
    linha(60, COR_AZUL);

    if (total_produtos == 0) {
        printf("\n  %sNenhum produto cadastrado.%s\n", COR_AMARELO, COR_RESET);
        pausar();
        return;
    }

    printf("\n  %-22s %10s %10s %10s %8s\n",
           "PRODUTO", "CUSTO", "VENDA", "MARGEM R$", "MARG. %");
    linha_simples(64, COR_CINZA);

    for (i = 0; i < total_produtos; i++) {
        double dif = produtos[i].preco - produtos[i].custo;
        double pct = (produtos[i].custo > 0)
                     ? (dif / produtos[i].custo) * 100.0
                     : 0.0;
        const char *cor_m = (dif >= 0) ? COR_VERDE : COR_VERMELHO;

        printf("  %-22s %10.2f %10.2f %s%10.2f %7.1f%%%s\n",
               produtos[i].nome,
               produtos[i].custo,
               produtos[i].preco,
               cor_m, dif, pct, COR_RESET);

        receita_total += produtos[i].preco * produtos[i].quantidade;
        custo_total   += produtos[i].custo * produtos[i].quantidade;
    }

    linha_simples(64, COR_CINZA);
    margem = (custo_total > 0)
             ? ((receita_total - custo_total) / custo_total) * 100.0
             : 0.0;
    printf("\n  %sMargem media do estoque: %s%.1f%%%s\n",
           COR_BRANCO, COR_AMARELO, margem, COR_RESET);
    pausar();
}

void fluxo_caixa(void) {
    int    i;
    double entradas = 0.0;
    double saidas   = 0.0; /* custo das mercadorias vendidas */

    limpar_tela();
    linha(60, COR_AZUL);
    printf("%s   [ FLUXO DE CAIXA / CONTAS A RECEBER ]%s\n", COR_AZUL, COR_RESET);
    linha(60, COR_AZUL);

    if (total_vendas == 0) {
        printf("\n  %sNenhuma movimentacao registrada.%s\n", COR_AMARELO, COR_RESET);
        pausar();
        return;
    }

    printf("\n  %-4s %-22s %12s %12s %12s\n",
           "#", "PRODUTO", "ENTRADA", "CMV", "LIQUIDO");
    linha_simples(66, COR_CINZA);

    for (i = 0; i < total_vendas; i++) {
        double cmv     = produtos[vendas[i].id_produto].custo * vendas[i].qtde;
        double liquido = vendas[i].total - cmv;
        entradas += vendas[i].total;
        saidas   += cmv;

        printf("  %-4d %-22s %12.2f %12.2f %s%12.2f%s\n",
               i+1,
               produtos[vendas[i].id_produto].nome,
               vendas[i].total,
               cmv,
               (liquido >= 0) ? COR_VERDE : COR_VERMELHO,
               liquido,
               COR_RESET);
    }

    linha_simples(66, COR_CINZA);
    printf("\n  %sTotal Entradas : %sR$ %.2f%s\n", COR_BRANCO, COR_VERDE,   entradas,          COR_RESET);
    printf("  %sTotal CMV      : %sR$ %.2f%s\n", COR_BRANCO, COR_AMARELO, saidas,             COR_RESET);
    printf("  %sSaldo Liquido  : %sR$ %.2f%s\n", COR_BRANCO, COR_VERDE,   entradas - saidas,  COR_RESET);
    printf("  %sSaldo em Caixa : %sR$ %.2f%s\n", COR_BRANCO, COR_CIANO,   caixa_total,        COR_RESET);
    pausar();
}

void menu_relatorios(void) {
    int op;
    do {
        limpar_tela();
        linha(54, COR_AZUL);
        printf("%s   [ RELATORIOS E DASHBOARD ]%s\n", COR_AZUL, COR_RESET);
        linha(54, COR_AZUL);
        printf("\n  %s[1]%s  Dashboard (Painel de Indicadores)\n", COR_CIANO, COR_RESET);
        printf("  %s[2]%s  Relatorio de Desempenho\n",             COR_CIANO, COR_RESET);
        printf("  %s[3]%s  Margem de Lucro\n",                     COR_CIANO, COR_RESET);
        printf("  %s[4]%s  Fluxo de Caixa / Contas a Receber\n",  COR_CIANO, COR_RESET);
        printf("  %s[0]%s  Voltar\n\n",                            COR_VERMELHO, COR_RESET);
        linha(54, COR_AZUL);
        printf("\n  %sEscolha: %s", COR_AMARELO, COR_RESET);
        if (scanf("%d", &op) != 1) op = -1;
        limpar_buffer();

        switch (op) {
            case 1: dashboard();           break;
            case 2: relatorio_desempenho();break;
            case 3: relatorio_margem();    break;
            case 4: fluxo_caixa();         break;
        }
    } while (op != 0);
}

/* =========================================================
 *  6. COMPRAS E SUPRIMENTOS
 * ========================================================= */
void novo_pedido_compra(void) {
    int     idx;
    Pedido *ped;

    if (total_pedidos >= MAX_PEDIDOS) {
        printf("\n  %sCapacidade maxima de pedidos atingida.%s\n", COR_VERMELHO, COR_RESET);
        pausar();
        return;
    }

    limpar_tela();
    linha(60, COR_AMARELO);
    printf("%s   [ NOVO PEDIDO DE COMPRA ]%s\n", COR_AMARELO, COR_RESET);
    linha(60, COR_AMARELO);

    exibir_tabela_estoque();

    ped = &pedidos[total_pedidos];
    ped->recebido = 0;

    printf("\n  %sNumero do produto: %s", COR_CIANO, COR_RESET);
    while (scanf("%d", &idx) != 1 || idx < 1 || idx > total_produtos) {
        printf("  %sInvalido: %s", COR_VERMELHO, COR_RESET);
        limpar_buffer();
    }
    limpar_buffer();
    ped->id_produto = idx - 1;

    printf("  %sQuantidade a comprar: %s", COR_CIANO, COR_RESET);
    while (scanf("%d", &ped->qtde_pedida) != 1 || ped->qtde_pedida <= 0) {
        printf("  %sInvalido: %s", COR_VERMELHO, COR_RESET);
        limpar_buffer();
    }
    limpar_buffer();

    printf("  %sPreco unitario (custo): R$ %s", COR_CIANO, COR_RESET);
    while (scanf("%lf", &ped->preco_unitario) != 1 || ped->preco_unitario < 0) {
        printf("  %sInvalido: R$ %s", COR_VERMELHO, COR_RESET);
        limpar_buffer();
    }
    limpar_buffer();

    total_pedidos++;
    printf("\n  %s Pedido #%d criado! Produto: %s | Qtde: %d | Total: R$ %.2f%s\n",
           COR_VERDE, total_pedidos,
           produtos[ped->id_produto].nome,
           ped->qtde_pedida,
           ped->qtde_pedida * ped->preco_unitario,
           COR_RESET);
    pausar();
}

void listar_pedidos(void) {
    int i;

    limpar_tela();
    linha(68, COR_AMARELO);
    printf("%s   [ PEDIDOS DE COMPRA ]%s\n", COR_AMARELO, COR_RESET);
    linha(68, COR_AMARELO);

    if (total_pedidos == 0) {
        printf("\n  %sNenhum pedido registrado.%s\n", COR_AMARELO, COR_RESET);
        pausar();
        return;
    }

    printf("\n  %-4s %-22s %8s %12s %12s %10s\n",
           "#", "PRODUTO", "QTDE", "PR.UNIT.", "TOTAL", "STATUS");
    linha_simples(72, COR_CINZA);

    for (i = 0; i < total_pedidos; i++) {
        const char *status = pedidos[i].recebido ? "RECEBIDO" : "PENDENTE";
        const char *cor_s  = pedidos[i].recebido ? COR_VERDE  : COR_AMARELO;
        printf("  %-4d %-22s %8d %12.2f %12.2f %s%10s%s\n",
               i+1,
               produtos[pedidos[i].id_produto].nome,
               pedidos[i].qtde_pedida,
               pedidos[i].preco_unitario,
               pedidos[i].qtde_pedida * pedidos[i].preco_unitario,
               cor_s, status, COR_RESET);
    }
    linha_simples(72, COR_CINZA);
    pausar();
}

void receber_pedido(void) {
    int    idx;
    Pedido *ped;

    limpar_tela();
    linha(60, COR_AMARELO);
    printf("%s   [ RECEBER PEDIDO ]%s\n", COR_AMARELO, COR_RESET);
    linha(60, COR_AMARELO);

    listar_pedidos();

    if (total_pedidos == 0) return;

    printf("  %sNumero do pedido: %s", COR_CIANO, COR_RESET);
    while (scanf("%d", &idx) != 1 || idx < 1 || idx > total_pedidos) {
        printf("  %sInvalido: %s", COR_VERMELHO, COR_RESET);
        limpar_buffer();
    }
    limpar_buffer();
    idx--;

    ped = &pedidos[idx];

    if (ped->recebido) {
        printf("\n  %sEste pedido ja foi recebido.%s\n", COR_AMARELO, COR_RESET);
        pausar();
        return;
    }

    ped->recebido = 1;
    produtos[ped->id_produto].quantidade += ped->qtde_pedida;
    produtos[ped->id_produto].custo       = ped->preco_unitario;

    printf("\n  %s Pedido recebido! Estoque de '%s' atualizado para %d unidades.%s\n",
           COR_VERDE,
           produtos[ped->id_produto].nome,
           produtos[ped->id_produto].quantidade,
           COR_RESET);
    pausar();
}

void menu_compras(void) {
    int op;
    do {
        limpar_tela();
        linha(54, COR_AMARELO);
        printf("%s   [ COMPRAS E SUPRIMENTOS ]%s\n", COR_AMARELO, COR_RESET);
        linha(54, COR_AMARELO);
        printf("\n  %s[1]%s  Novo Pedido de Compra\n",  COR_CIANO, COR_RESET);
        printf("  %s[2]%s  Listar Pedidos\n",            COR_CIANO, COR_RESET);
        printf("  %s[3]%s  Receber Pedido\n",            COR_CIANO, COR_RESET);
        printf("  %s[0]%s  Voltar\n\n",                  COR_VERMELHO, COR_RESET);
        linha(54, COR_AMARELO);
        printf("\n  %sEscolha: %s", COR_AMARELO, COR_RESET);
        if (scanf("%d", &op) != 1) op = -1;
        limpar_buffer();

        switch (op) {
            case 1: novo_pedido_compra(); break;
            case 2: listar_pedidos();     break;
            case 3: receber_pedido();     break;
        }
    } while (op != 0);
}

/* =========================================================
 *  SAIDA
 * ========================================================= */
void tela_saida(void) {
    limpar_tela();
    linha(54, COR_AZUL);
    printf("\n  %s Obrigado por usar o Sistema de Vendas!%s\n", COR_CIANO, COR_RESET);
    printf("  %s Encerrando o programa...%s\n\n", COR_CINZA, COR_RESET);
    linha(54, COR_AZUL);
}

/* =========================================================
 *  MAIN
 * ========================================================= */
int main(void) {
    int opcao;

    tela_login();

    do {
        opcao = menu_principal();

        switch (opcao) {
            case 1: registrar_produtos(); break;
            case 2: ver_estoque();        break;
            case 3: realizar_venda();     break;
            case 4: menu_clientes();      break;
            case 5: menu_relatorios();    break;
            case 6: menu_compras();       break;
            case 0: tela_saida();         break;
            default:
                printf("\n  %sOpcao invalida! Escolha entre 0 e 6.%s\n",
                       COR_VERMELHO, COR_RESET);
                pausar();
                break;
        }
    } while (opcao != 0);

    return 0;
}

