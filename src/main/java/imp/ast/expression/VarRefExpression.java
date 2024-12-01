package imp.ast.expression;

import imp.ast.ASTNode;
import imp.ast.variable.Identifier;

import java.util.List;

public final class VarRefExpression extends ReferenceExpression {
    private final Identifier variableName;

    public VarRefExpression(Identifier variableName) {
        this.variableName = variableName;
    }

    public Identifier variableName() {
        return variableName;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof VarRefExpression expr) {
            return variableName.equals(expr.variableName);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return variableName.hashCode();
    }

    @Override
    public String toString() {
        return variableName.toString();
    }

    @Override
    public List<ASTNode> getChildren() {
        return List.of(variableName);
    }
}
