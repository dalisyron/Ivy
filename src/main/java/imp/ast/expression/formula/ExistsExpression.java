package imp.ast.expression.formula;

import imp.ast.expression.Expression;
import imp.ast.variable.Identifier;

// Define the ExistsExpr subclass for "exists" quantifier
public final class ExistsExpression extends QuantifiedExpression {

    public ExistsExpression(Identifier variable, Expression expression) {
        super("exists", variable, expression);
    }

    @Override
    public boolean equals(Object obj) {
        return super.equals(obj);
    }

    @Override
    public int hashCode() {
        return super.hashCode();
    }
}
