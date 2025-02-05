package imp.ast.statement;

import imp.ast.ASTVisitor;
import imp.ast.expression.Expression;
import imp.ast.expression.ReferenceExpression;

import java.util.Objects;

public final class AssignStatement extends Statement {

    private final ReferenceExpression lhs;
    private final Expression expression;

    public AssignStatement(ReferenceExpression lhs, Expression expression) {
        this.lhs = lhs;
        this.expression = expression;
    }

    public ReferenceExpression lhs() {
        return lhs;
    }

    public Expression expression() {
        return expression;
    }


    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        AssignStatement that = (AssignStatement) o;

        if (!Objects.equals(lhs, that.lhs)) return false;
        return Objects.equals(expression, that.expression);
    }

    @Override
    public int hashCode() {
        return Objects.hash(lhs, expression);
    }

    @Override
    public void accept(ASTVisitor v) {
        v.visit(this);
    }
}
