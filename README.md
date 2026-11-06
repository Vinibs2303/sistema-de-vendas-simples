# Sistema de Vendas em C

Projeto acadêmico desenvolvido em linguagem C para simular o gerenciamento de uma pequena operação comercial por meio de um menu no terminal.

## Funcionalidades

- Cadastro de produtos e controle de estoque;
- Registro de vendas, com cálculo de descontos, impostos e troco;
- Cadastro de clientes e consulta ao histórico de compras;
- Relatórios de desempenho, margem e fluxo de caixa;
- Dashboard gráfico no Windows;
- Registro e recebimento de pedidos de compra.

## Tecnologias

- Linguagem C;
- Dev-C++;
- API do Windows (para o dashboard gráfico).

## Como executar

1. Abra `Projeto1.dev` no Dev-C++.
2. Compile e execute o projeto.

Também é possível compilar pelo terminal usando GCC/MinGW no Windows:

```bash
gcc src/Sistema_de_Vendas_Def.c -o sistema.exe -lgdi32 -lm
```

Depois, execute `sistema.exe`.

## Acesso de demonstração

- **Usuário:** `admin`
- **Senha:** `1234`

Essas credenciais estão definidas diretamente no código e servem apenas para demonstração. Não utilize esse modelo de autenticação em um sistema real.

## Observações

- Os dados são armazenados em memória durante a execução. Ao fechar o programa, os registros cadastrados são perdidos.
- O dashboard gráfico utiliza recursos da API do Windows; por isso, essa parte é específica para Windows.
- Este projeto tem finalidade acadêmica e demonstrativa.
