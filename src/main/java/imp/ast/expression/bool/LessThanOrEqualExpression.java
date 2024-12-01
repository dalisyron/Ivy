package imp.ast.expression.bool;

import imp.ast.ASTNode;
import imp.ast.expression.Expression;

import java.util.List;

public class LessThanOrEqualExpression extends Expression implements BoolExpectedType {
    private final Expression left;
    private final Expression right;

    public LessThanOrEqualExpression(Expression left, Expression right) {
        this.left = left;
        this.right = right;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof LessThanOrEqualExpression lessThanOrEqualExpr) {
            return left.equals(lessThanOrEqualExpr.left) && right.equals(lessThanOrEqualExpr.right);
        }
        return false;
    }

    public Expression left() {
        return left;
    }

    public Expression right() {
        return right;
    }

    @Override
    public int hashCode() {
        return 31 * left.hashCode() + right.hashCode();
    }

    @Override
    public String toString() {
        return left + " <= " + right;
    }

    @Override
    public List<ASTNode> getChildren() {
        return List.of(left, right);
    }
}
