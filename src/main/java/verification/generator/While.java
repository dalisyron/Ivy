package verification.generator;

import com.microsoft.z3.BoolExpr;
import com.microsoft.z3.Context;
import imp.ast.expression.Expression;
import imp.ast.InvariantList;
import imp.ast.statement.BlockStatement;
import imp.ast.statement.WhileStatement;
import interpreter.Z3Interpreter;

public class While implements VerificationConditionProvider<WhileStatement> {
    private static While instance;

    private While() {
    }

    @Override
    public BoolExpr awp(Context ctx, WhileStatement whileStatement, BoolExpr Q) {
        InvariantList invariants = whileStatement.invariants();

        return Z3Interpreter.create(ctx).interpret(invariants);
    }

    @Override
    public BoolExpr avc(Context ctx, WhileStatement whileStatement, BoolExpr Q) {
        // $A V C\left(S^{\prime}, Q\right)=A V C(S, I) \wedge(I \wedge C \rightarrow a w p(S, I)) \wedge(I \wedge \neg C \rightarrow Q)$
        Z3Interpreter interpreter = Z3Interpreter.create(ctx);

        InvariantList invariants = whileStatement.invariants();

        BoolExpr I = interpreter.interpret(invariants);

        BlockStatement body = whileStatement.body();
        Expression condition = whileStatement.condition();
        BoolExpr condExpr = (BoolExpr) interpreter.interpret(condition);

        BoolExpr A = AVC.getInstance().avc(ctx, body, I);
        BoolExpr B = ctx.mkImplies(ctx.mkAnd(I, condExpr), AWP.getInstance().awp(ctx, body, I));
        BoolExpr C = ctx.mkImplies(ctx.mkAnd(I, ctx.mkNot(condExpr)), Q);

        return ctx.mkAnd(A, B, C);
    }

    public static While getInstance() {
        if (instance == null) {
            instance = new While();
        }
        return instance;
    }
}