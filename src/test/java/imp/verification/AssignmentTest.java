package imp.verification;

import org.junit.Test;

import com.microsoft.z3.BoolExpr;
import com.microsoft.z3.Context;
import com.microsoft.z3.IntExpr;
import com.microsoft.z3.Solver;
import com.microsoft.z3.Status;

import imp.ast.expression.VarRefExpression;
import imp.ast.expression.constant.integer.IntExpression;
import imp.ast.statement.AssignStatement;

import imp.ast.variable.*;

import org.junit.Assert;


public class AssignmentTest {
    @Test
    public void AssignIntTest() {
        Context ctx = new Context();
        AssignStatement statement = new AssignStatement(new VarRefExpression(new Identifier("x")), new IntExpression(1));
        IntExpr x = ctx.mkIntConst("x");
        BoolExpr Q = ctx.mkEq(x, ctx.mkInt(1));
        BoolExpr awp = Assignment.getInstance().awp(ctx, statement, Q);
        BoolExpr avc = Assignment.getInstance().avc(ctx, statement, Q);
        Solver solver = ctx.mkSolver();
        solver.add(ctx.mkNot(ctx.mkAnd(awp, avc)));
        Status status = solver.check();
        Assert.assertEquals(Status.UNSATISFIABLE, status);
    }

}