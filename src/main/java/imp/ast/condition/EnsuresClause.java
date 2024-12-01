package imp.ast.condition;

import imp.ast.ASTNode;
import imp.ast.expression.Expression;

import java.util.List;

public record EnsuresClause(Expression expression) implements ConditionClause {

    @Override
    public Expression expr() {
        return expression;
    }

    @Override
    public String toString() {
        return "ensures " + expression.toString();
    }

    @Override
    public List<ASTNode> getChildren() {
        return List.of(expression);
    }
}
