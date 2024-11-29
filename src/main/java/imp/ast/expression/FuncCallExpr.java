package imp.ast.expression;

import imp.ast.variable.Identifier;

import java.util.List;

public final class FuncCallExpr extends Expr {

    private final Identifier functionName;
    private final List<Expr> arguments;

    public FuncCallExpr(Identifier functionName, List<Expr> arguments) {
        this.functionName = functionName;
        this.arguments = arguments;
    }

    public Identifier functionName() {
        return functionName;
    }

    public List<Expr> arguments() {
        return arguments;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof FuncCallExpr that)) return false;

        if (!functionName.equals(that.functionName)) return false;
        return arguments.equals(that.arguments);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(functionName).append("(");
        for (int i = 0; i < arguments.size(); i++) {
            sb.append(arguments.get(i).toString());
            if (i < arguments.size() - 1) {
                sb.append(", ");
            }
        }
        sb.append(")");
        return sb.toString();
    }
}
